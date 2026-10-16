#!/usr/bin/env python3
"""
send_pose_target.py — 控制器IK + OMPL关节空间规划

流程:
  1. 控制器 InverseKin(目标位姿) → 精准关节角 J_target
  2. JointConstraint(J_target, tol=0.0001rad) → OMPL 关节空间规划
  3. /execute_trajectory → action_move_server → ServoJ

用法:
  ros2 run dobot_moveit send_pose_target.py -- x y z rx ry rz [--plan-only]
"""

import math, os, sys, threading, time
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from moveit_msgs.action import MoveGroup, ExecuteTrajectory
from moveit_msgs.msg import (
    Constraints, JointConstraint, WorkspaceParameters,
    DisplayTrajectory, RobotTrajectory,
)
from dobot_msgs_v4.srv import InverseKin, GetAngle


def normalize_deg(target, current):
    """以 current 为基准，将 target 归算到最近等效角度，保证最短物理路径。"""
    delta = math.fmod(target - current, 360.0)
    if delta > 180.0:
        delta -= 360.0
    elif delta < -180.0:
        delta += 360.0
    return current + delta


def main():
    rclpy.init()
    args = sys.argv[1:]
    plan_only = '--plan-only' in args
    if plan_only: args.remove('--plan-only')
    if len(args) != 6:
        print(f"用法: {sys.argv[0]} x y z rx ry rz [--plan-only]"); sys.exit(1)

    x, y, z = map(float, args[0:3])
    rx, ry, rz = map(float, args[3:6])
    robot_type = os.getenv("DOBOT_TYPE", "cr5")
    group_name = f"{robot_type}_group"
    joint_names = [f"joint{i+1}" for i in range(6)]

    node = rclpy.create_node('send_pose_target_client')
    logger = node.get_logger()
    logger.info(f"机器人: {robot_type}")
    logger.info(f"目标: pos=({x:.4f},{y:.4f},{z:.4f})m rot=({rx:.1f},{ry:.1f},{rz:.1f})°")

    # ── 1. 控制器 InverseKin → 精准关节角 ──
    ang = node.create_client(GetAngle, '/dobot_bringup_ros2/srv/GetAngle')
    cur_deg = None
    if ang.wait_for_service(timeout_sec=3.0):
        time.sleep(0.1); rclpy.spin_once(node, timeout_sec=0.05)
        af = ang.call_async(GetAngle.Request())
        rclpy.spin_until_future_complete(node, af, timeout_sec=3.0)
        if af.done() and af.result() and af.result().res == 0:
            cur_deg = [float(v) for v in af.result().robot_return.strip('{}').split(',') if v.strip()][:6]
    if not cur_deg: logger.error("无法获取当前关节"); sys.exit(1)

    ik = node.create_client(InverseKin, '/dobot_bringup_ros2/srv/InverseKin')
    if not ik.wait_for_service(timeout_sec=5.0):
        logger.error("控制器 IK 不可达"); sys.exit(1)

    req = InverseKin.Request()
    req.x, req.y, req.z = x * 1000.0, y * 1000.0, z * 1000.0
    req.rx, req.ry, req.rz = rx, ry, rz
    req.use_joint_near = '1'
    req.joint_near = '{' + ','.join(str(v) for v in cur_deg) + '}'
    req.user, req.tool = '0', '0'

    fut = ik.call_async(req)
    rclpy.spin_until_future_complete(node, fut, timeout_sec=5.0)
    if not fut.done() or not fut.result() or fut.result().res != 0:
        logger.error("控制器 IK 失败"); sys.exit(1)
    raw_tgt = [float(v) for v in fut.result().robot_return.strip('{}').split(',') if v.strip()]
    tgt_deg = [normalize_deg(raw_tgt[i], cur_deg[i]) for i in range(len(raw_tgt))]
    logger.info(f"控制器 IK(deg): {[round(v,4) for v in tgt_deg]}")

    # ── 2. 关节约束 → OMPL 关节空间规划 ──
    move = ActionClient(node, MoveGroup, "/move_action")
    if not move.wait_for_server(timeout_sec=10.0):
        logger.error("move_group 不可达"); sys.exit(1)

    c = Constraints()
    for i, jn in enumerate(joint_names):
        jc = JointConstraint()
        jc.joint_name = jn
        jc.position = math.radians(tgt_deg[i])
        jc.tolerance_above = 0.0001
        jc.tolerance_below = 0.0001
        jc.weight = 1.0
        c.joint_constraints.append(jc)

    goal = MoveGroup.Goal()
    goal.request.group_name = group_name
    goal.request.num_planning_attempts = 1
    goal.request.allowed_planning_time = 3.0
    goal.request.goal_constraints.append(c)
    goal.planning_options.plan_only = plan_only
    goal.request.workspace_parameters = WorkspaceParameters()
    goal.request.workspace_parameters.header.frame_id = 'base_link'
    goal.request.workspace_parameters.min_corner.x = -1.5
    goal.request.workspace_parameters.min_corner.y = -1.5
    goal.request.workspace_parameters.min_corner.z = -0.5
    goal.request.workspace_parameters.max_corner.x = 1.5
    goal.request.workspace_parameters.max_corner.y = 1.5
    goal.request.workspace_parameters.max_corner.z = 1.5

    logger.info("OMPL 关节空间规划...")
    mf = move.send_goal_async(goal)
    rclpy.spin_until_future_complete(node, mf, timeout_sec=30.0)
    if not mf.done(): logger.error("超时"); sys.exit(1)
    gh = mf.result()
    if not gh.accepted: logger.error("被拒绝"); sys.exit(1)
    rf = gh.get_result_async()
    rclpy.spin_until_future_complete(node, rf, timeout_sec=30.0)
    if not rf.done(): logger.error("结果超时"); sys.exit(1)
    wr = rf.result()
    if wr.result.error_code.val != 1:
        logger.error(f"规划失败 code={wr.result.error_code.val}"); sys.exit(1)

    traj = wr.result.planned_trajectory
    pts = traj.joint_trajectory.points
    jn = traj.joint_trajectory.joint_names
    logger.info(f"规划成功! {len(pts)} 个轨迹点")
    if pts:
        logger.info(f"起点(deg): {dict(zip(jn, [round(math.degrees(v),2) for v in pts[0].positions]))}")
    if len(pts) > 1:
        logger.info(f"终点(deg): {dict(zip(jn, [round(math.degrees(v),4) for v in pts[-1].positions]))}")

    if plan_only:
        # 后台续发轨迹到 RViz，规划阶段持续显示
        disp = DisplayTrajectory()
        disp.model_id = f'{robot_type}_robot'
        rt = RobotTrajectory()
        rt.joint_trajectory = traj.joint_trajectory
        disp.trajectory.append(rt)
        pub = node.create_publisher(DisplayTrajectory, '/display_planned_path', 10)
        stop = threading.Event()
        def keep():
            while not stop.is_set() and rclpy.ok():
                pub.publish(disp); stop.wait(timeout=0.8)
        t = threading.Thread(target=keep, daemon=True); t.start()

        print(f"\n{'='*50}\n  白色轨迹已显示  按 Enter 执行  |  Ctrl+C 取消\n{'='*50}")
        try:
            input()
            stop.set(); t.join(0.5)
        except KeyboardInterrupt:
            stop.set()
            logger.info("已取消"); sys.exit(0)

    # ── 3. 执行 ──
    logger.info("执行中...")
    ex = ActionClient(node, ExecuteTrajectory, "/execute_trajectory")
    if not ex.wait_for_server(timeout_sec=10.0):
        logger.error("execute_trajectory 不可达"); sys.exit(1)
    eg = ExecuteTrajectory.Goal(); eg.trajectory = traj
    ef = ex.send_goal_async(eg)
    rclpy.spin_until_future_complete(node, ef, timeout_sec=10.0)
    if not ef.done(): logger.error("超时"); sys.exit(1)
    eh = ef.result()
    if not eh.accepted: logger.error("被拒绝"); sys.exit(1)
    erf = eh.get_result_async()
    rclpy.spin_until_future_complete(node, erf, timeout_sec=30.0)
    if not erf.done(): logger.error("执行超时"); sys.exit(1)
    if erf.result().result.error_code.val == 1:
        logger.info("✓ 执行完成")
    else:
        logger.error("执行失败")

    node.destroy_node(); rclpy.shutdown()


if __name__ == "__main__":
    main()
