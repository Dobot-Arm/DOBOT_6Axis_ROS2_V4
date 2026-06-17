#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
@author FTX
@date 2025 / 03 / 03

MoveIt FollowJointTrajectory Action Server for Dobot robots.
- Synchronous execute_callback (ROS2 ActionServer handles it in the default executor).
- ReentrantCallbackGroup ensures cancel callback can interrupt execution.
- On cancel: Stop()+ClearError() on the physical robot.
'''

import time
import threading
import rclpy
from rclpy.action import ActionServer, CancelResponse, GoalResponse
from rclpy.callback_groups import ReentrantCallbackGroup
from rclpy.node import Node
from control_msgs.action import FollowJointTrajectory
from dobot_msgs_v4.srv import (
    EnableRobot, ServoJ, GetAngle, Stop, ClearError
)
import os


class FollowJointTrajectoryServer(Node):

    def __init__(self):
        super().__init__('dobot_group_controller')
        name = os.getenv("DOBOT_TYPE", "cr5")

        # 声明超时参数（默认120秒）
        self.declare_parameter('trajectory_execution_timeout', 120.0)
        self._timeout = self.get_parameter('trajectory_execution_timeout').value

        self._action_server = ActionServer(
            self, FollowJointTrajectory,
            f'/{name}_group_controller/follow_joint_trajectory',
            execute_callback=self._on_execute,
            callback_group=ReentrantCallbackGroup(),
            cancel_callback=self._on_cancel,
            goal_callback=self._on_goal,
        )
        self.get_logger().info("FollowJointTrajectory Action Server is ready")

        self._srv_enable  = self.create_client(EnableRobot, '/dobot_bringup_ros2/srv/EnableRobot')
        self._srv_servoj  = self.create_client(ServoJ,     '/dobot_bringup_ros2/srv/ServoJ')
        self._srv_angle   = self.create_client(GetAngle,   '/dobot_bringup_ros2/srv/GetAngle')
        self._srv_stop    = self.create_client(Stop,       '/dobot_bringup_ros2/srv/Stop')
        self._srv_clr_err = self.create_client(ClearError, '/dobot_bringup_ros2/srv/ClearError')

        self._cancel_flag = threading.Event()

        while not self._srv_enable.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('waiting for EnableRobot service ...')

    # ── 工具 ──────────────────────────────────────────
    @staticmethod
    def _result(code):
        r = FollowJointTrajectory.Result()
        r.error_code = code
        return r

    def _call(self, client, req, timeout=3.0):
        future = client.call_async(req)
        rclpy.spin_until_future_complete(self, future, timeout_sec=timeout)
        return future.done() and future.result() is not None and future.result().res == 0

    # ── Goal / Cancel ─────────────────────────────────
    def _on_goal(self, goal_request):
        self.get_logger().info("New goal accepted")
        return GoalResponse.ACCEPT

    def _on_cancel(self, goal_handle):
        self.get_logger().warn("CANCEL — stopping robot")
        self._cancel_flag.set()
        self._do_stop()
        return CancelResponse.ACCEPT

    def _do_stop(self):
        self._call(self._srv_stop,    Stop.Request(),       timeout=3.0)
        self._call(self._srv_clr_err, ClearError.Request(), timeout=3.0)

    # ── execute_callback (同步) ────────────────────────
    def _on_execute(self, goal_handle):
        self.get_logger().info("Received a new trajectory goal!")
        self._cancel_flag.clear()

        traj = goal_handle.request.trajectory
        if not traj.points:
            goal_handle.abort()
            return self._result(FollowJointTrajectory.Result.INVALID_GOAL)

        # 使用初始化时读取的超时时间
        t0 = time.time()

        failed = self._run_trajectory(traj, self._timeout, t0)

        self._cancel_flag.clear()

        if goal_handle.is_cancel_requested:
            goal_handle.canceled()
            return self._result(FollowJointTrajectory.Result.SUCCESSFUL)
        elif failed:
            goal_handle.abort()
            return self._result(FollowJointTrajectory.Result.GOAL_TOLERANCE_VIOLATED)
        else:
            goal_handle.succeed()
            return self._result(FollowJointTrajectory.Result.SUCCESSFUL)

    # ── 轨迹执行 ──────────────────────────────────────
    def _run_trajectory(self, traj, timeout, t0):
        pts = self._parse_points(traj)
        if pts is None:
            return True

        n = len(pts)
        dur = pts[-1][1]
        self.get_logger().info(f"执行轨迹: {n}点 / {dur:.2f}s (超时: {timeout}s)")

        for i, (deg, t_target) in enumerate(pts):
            # 检查超时
            if time.time() - t0 > timeout:
                self.get_logger().warn(f"执行超时 ({timeout}s)")
                self._do_stop()  # 阻塞停止，确保机器人停止
                return True  # 返回失败

            if self._cancel_flag.is_set():
                self.get_logger().info(f"取消-点{i}")
                return False

            dt = 0.1 if i == 0 else t_target - pts[i - 1][1]
            dt = max(0.05, min(dt, 3600.0))
            self._wait_until(t_target, t0)

            if not self._call_servoj(deg, dt):
                self.get_logger().error(f"ServoJ@{i} 失败")
                self._do_stop()
                return True

            if i % 10 == 0 or i == n - 1:
                self.get_logger().info(f"点{i}/{n-1}: dt={dt:.3f}s T={time.time()-t0:.2f}s")

        # 检查最终超时
        if time.time() - t0 > timeout:
            self.get_logger().warn(f"执行超时 ({timeout}s)")
            self._do_stop()  # 阻塞停止，确保机器人停止
            return True

        last_dt = pts[-1][1] - pts[-2][1] if n > 1 else 0.1
        rem = dur + last_dt * 0.5 - (time.time() - t0)
        if rem > 0:
            self._wait_until(dur + last_dt * 0.5, t0)

        self._verify_position(pts[-1][0])
        self.get_logger().info("轨迹执行完成")
        return False

    def _parse_points(self, traj):
        pts = []
        for i, pt in enumerate(traj.points):
            deg = [180.0 * r / 3.141592653589793 for r in pt.positions]
            ts = pt.time_from_start.sec + pt.time_from_start.nanosec / 1e9
            pts.append((deg, ts))
            if i % 10 == 0 or i == len(traj.points) - 1:
                self.get_logger().info(f"Pt {i}: {deg} T={ts:.3f}s")

        if len(pts) < 2:
            self.get_logger().error("轨迹点数不足")
            return None

        total = pts[-1][1]
        if total <= 0.001:
            n = len(pts)
            dur = max(5.0, n * 0.1)
            self.get_logger().warn(f"缺时间戳 → 均匀分配 {dur:.1f}s")
            pts = [(p[0], dur * i / (n - 1)) for i, p in enumerate(pts)]

        return pts

    def _wait_until(self, target_time, t0):
        while True:
            if self._cancel_flag.is_set():
                return
            rem = target_time - (time.time() - t0)
            if rem <= 0.002:
                break
            rclpy.spin_once(self, timeout_sec=min(rem, 0.01))

    def _call_servoj(self, deg, t):
        req = ServoJ.Request()
        req.a, req.b, req.c = float(deg[0]), float(deg[1]), float(deg[2])
        req.d, req.e, req.f = float(deg[3]), float(deg[4]), float(deg[5])
        req.param_value = [f"t={max(0.004, t)}"]
        return self._call(self._srv_servoj, req, timeout=max(3.0, t * 10))

    def _verify_position(self, target_deg):
        if not self._srv_angle.service_is_ready():
            return
        future = self._srv_angle.call_async(GetAngle.Request())
        rclpy.spin_until_future_complete(self, future, timeout_sec=0.5)
        if not future.done() or not future.result() or future.result().res != 0:
            return
        try:
            s = future.result().robot_return.strip().strip('{}')
            vals = [float(x) for x in s.split(',') if x.strip()]
            if len(vals) >= 6:
                errs = [abs(vals[i] - target_deg[i]) for i in range(6)]
                self.get_logger().info(f"到位检查: max_err={max(errs):.3f}°")
        except Exception:
            pass


def main(args=None):
    rclpy.init(args=args)
    node = FollowJointTrajectoryServer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
