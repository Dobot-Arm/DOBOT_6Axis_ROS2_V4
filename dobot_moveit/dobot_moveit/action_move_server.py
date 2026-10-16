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
from rclpy.executors import MultiThreadedExecutor
from rclpy.node import Node
from control_msgs.action import FollowJointTrajectory
from dobot_msgs_v4.srv import (
    EnableRobot, ServoJ, GetAngle, Stop, ClearError
)


class FollowJointTrajectoryServer(Node):

    def __init__(self):
        super().__init__('action_move_server')

        self.declare_parameter('robot_type', 'cr5')
        self.declare_parameter('trajectory_execution_timeout', 120.0)
        # ServoJ 同步策略（保证"发送间隔 < 指令运行时间 t"，避免段末等目标造成顿挫）：
        #   extend_t     = 按计划时刻发送，把 t 抬高一个提前量 lead（send 间隔 = 段时长，t = 段时长 + lead）
        #   advance_send = t 保持段时长，把发送时刻提前 lead（send 间隔 = 段时长 - lead，t = 段时长）
        self.declare_parameter('servoj_sync_mode', 'extend_t')
        # 提前量 lead = max(段时长 × servoj_lead_percent%, servoj_lead_min_sec)
        #   比例项：如 5.0 表示"比 t 小 5%"；绝对项：如 0.005 表示固定 5ms 下限
        self.declare_parameter('servoj_lead_percent', 5.0)
        self.declare_parameter('servoj_lead_min_sec', 0.005)

        self._robot_type = self.get_parameter('robot_type').value
        self._timeout = self.get_parameter('trajectory_execution_timeout').value
        self._sync_mode = str(self.get_parameter('servoj_sync_mode').value)
        self._lead_percent = float(self.get_parameter('servoj_lead_percent').value)
        self._lead_min_sec = float(self.get_parameter('servoj_lead_min_sec').value)
        if self._sync_mode not in ('extend_t', 'advance_send'):
            self.get_logger().warning(
                f"未知 servoj_sync_mode='{self._sync_mode}'，回退为 extend_t")
            self._sync_mode = 'extend_t'
        self.get_logger().info(
            f"ServoJ 同步策略={self._sync_mode}, "
            f"lead=max({self._lead_percent:.1f}% × 段时长, {self._lead_min_sec * 1000.0:.1f}ms)"
        )

        action_name = f'{self._robot_type}_group_controller/follow_joint_trajectory'
        self._action_server = ActionServer(
            self, FollowJointTrajectory,
            action_name,
            execute_callback=self._on_execute,
            callback_group=ReentrantCallbackGroup(),
            cancel_callback=self._on_cancel,
            goal_callback=self._on_goal,
        )
        self.get_logger().info(f"FollowJointTrajectory Action Server is ready on /{action_name}")

        self._srv_enable  = self.create_client(EnableRobot, '/dobot_bringup_ros2/srv/EnableRobot')
        self._srv_servoj  = self.create_client(ServoJ,     '/dobot_bringup_ros2/srv/ServoJ')
        self._srv_angle   = self.create_client(GetAngle,   '/dobot_bringup_ros2/srv/GetAngle')
        self._srv_stop    = self.create_client(Stop,       '/dobot_bringup_ros2/srv/Stop')
        self._srv_clr_err = self.create_client(ClearError, '/dobot_bringup_ros2/srv/ClearError')

        self._cancel_flag = threading.Event()
        self._connected = False

        self.get_logger().info('Waiting for EnableRobot service (timeout 30s)...')
        if self._srv_enable.wait_for_service(timeout_sec=30.0):
            self._connected = True
            self.get_logger().info('EnableRobot service connected')
        else:
            self.get_logger().warning('EnableRobot service not available, MoveIt will run without hardware')

    # ── 工具 ──────────────────────────────────────────
    @staticmethod
    def _result(code):
        r = FollowJointTrajectory.Result()
        r.error_code = code
        return r

    def _call(self, client, req, timeout=3.0):
        if not self._connected:
            self.get_logger().warning('Hardware not connected, skipping service call')
            return False
        if not client.service_is_ready():
            self.get_logger().warning(f'Service not ready: {client.srv_name}')
            return False
        future = client.call_async(req)
        evt = threading.Event()
        future.add_done_callback(lambda _: evt.set())
        if not evt.wait(timeout=timeout):
            self.get_logger().error(f'Service call timeout: {client.srv_name}')
            return False
        return future.result() is not None and future.result().res == 0

    # ── Goal / Cancel ─────────────────────────────────
    def _on_goal(self, goal_request):
        self.get_logger().info("New goal accepted")
        return GoalResponse.ACCEPT

    def _on_cancel(self, goal_handle):
        self.get_logger().warning("CANCEL — stopping robot")
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

        if not self._connected:
            self.get_logger().error('Hardware not connected, cannot execute trajectory')
            goal_handle.abort()
            return self._result(FollowJointTrajectory.Result.GOAL_TOLERANCE_VIOLATED)

        traj = goal_handle.request.trajectory
        if not traj.points:
            goal_handle.abort()
            return self._result(FollowJointTrajectory.Result.INVALID_GOAL)

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

        last_cmd_t = 0.1
        last_send = 0.0     # 最近一次指令的计划发送时刻
        sched = 0.0         # advance_send 模式自维护的压缩时间轴
        for i, (deg, t_target) in enumerate(pts):
            # 检查超时
            if time.time() - t0 > timeout:
                self.get_logger().warning(f"执行超时 ({timeout}s)")
                self._do_stop()  # 阻塞停止，确保机器人停止
                return True  # 返回失败

            if self._cancel_flag.is_set():
                self.get_logger().info(f"取消-点{i}")
                return False

            # 段时长：首点取真实首段（原实现硬编码 0.1，与实际首段不符）
            if i == 0:
                seg_dt = (pts[1][1] - t_target) if n > 1 else 0.1
            else:
                seg_dt = t_target - pts[i - 1][1]
            seg_dt = max(0.004, seg_dt)

            lead = self._servoj_lead(seg_dt)
            if self._sync_mode == 'advance_send':
                # 方案B：t 保持段时长，压缩发送周期 → 间隔 = seg_dt - lead < t
                # 注意必须用"累计时间轴"，直接用 t_target - lead 会因常数提前量前后抵消而无效
                if i > 0:
                    sched += seg_dt - lead
                send_at, t_cmd, interval = sched, seg_dt, seg_dt - lead
            else:
                # 方案A：按计划时刻发送，抬高 t → 间隔 = seg_dt < t = seg_dt + lead
                send_at, t_cmd, interval = t_target, seg_dt + lead, seg_dt

            self._wait_until(send_at, t0)
            last_send = send_at
            last_cmd_t = t_cmd

            if not self._call_servoj(deg, t_cmd):
                self.get_logger().error(f"ServoJ@{i} 失败")
                self._do_stop()
                return True

            if i % 10 == 0 or i == n - 1:
                self.get_logger().info(
                    f"点{i}/{n-1}: 间隔={interval * 1000.0:.1f}ms "
                    f"t={t_cmd * 1000.0:.1f}ms (lead={lead * 1000.0:.1f}ms) "
                    f"T={time.time()-t0:.2f}s")

        # 检查最终超时
        if time.time() - t0 > timeout:
            self.get_logger().warning(f"执行超时 ({timeout}s)")
            self._do_stop()  # 阻塞停止，确保机器人停止
            return True

        # 等最后一段完整走完（ServoJ t=last_cmd_t）再做到位检查
        # 原实现只等 0.5×段时长，机器人尚未到位就开始检查，误差偏大
        tail = last_send + last_cmd_t
        if tail - (time.time() - t0) > 0:
            self._wait_until(tail, t0)

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
            self.get_logger().warning(f"缺时间戳 → 均匀分配 {dur:.1f}s")
            pts = [(p[0], dur * i / (n - 1)) for i, p in enumerate(pts)]

        return pts

    def _wait_until(self, target_time, t0):
        while True:
            if self._cancel_flag.is_set():
                return
            rem = target_time - (time.time() - t0)
            if rem <= 0.002:
                break
            time.sleep(max(0.001, min(rem, 0.01)))

    def _servoj_lead(self, seg_dt):
        """提前量(秒) = max(段时长 × 百分比, 绝对下限)，并限制不超过半段。

        extend_t     : 发送间隔 = seg_dt，      t = seg_dt + lead → 间隔 < t
        advance_send : 发送间隔 = seg_dt - lead，t = seg_dt         → 间隔 < t
        """
        lead = max(self._lead_percent / 100.0 * seg_dt, self._lead_min_sec)
        # 上限半段：避免压缩时间轴出现非正间隔，或 t 被抬得过多
        return min(lead, 0.5 * seg_dt)

    def _call_servoj(self, deg, t):
        req = ServoJ.Request()
        req.a, req.b, req.c = float(deg[0]), float(deg[1]), float(deg[2])
        req.d, req.e, req.f = float(deg[3]), float(deg[4]), float(deg[5])
        # 保留 5 位小数：避免浮点误差产生 t=0.09999999999999998 这类串
        req.param_value = [f"t={round(max(0.004, t), 5)}"]
        # 单次 ServoJ 正常往返仅几十 ms；卡住时最多阻塞 t+0.5s，
        # 原实现 max(3.0, t*10) 可达 3s，会让机器人在段中停等造成顿挫
        return self._call(self._srv_servoj, req, timeout=max(0.5, t + 0.5))

    def _verify_position(self, target_deg):
        if not self._srv_angle.service_is_ready():
            return
        future = self._srv_angle.call_async(GetAngle.Request())
        evt = threading.Event()
        future.add_done_callback(lambda _: evt.set())
        if not evt.wait(timeout=0.5) or not future.result() or future.result().res != 0:
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
    executor = MultiThreadedExecutor()
    executor.add_node(node)
    try:
        executor.spin()
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
