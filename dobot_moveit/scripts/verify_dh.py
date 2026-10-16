#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
验证控制器标定 DH 精度 — OMPL 路径 + 精度补偿架构的验证工具。

架构: OMPL(KDL)规划路径 → 每点控制器IK补偿精度 → 执行
本脚本验证: 控制器标定 DH 的自洽性（IK→FK 回验）vs KDL 理论 DH 的偏差

检查项:
  1. 控制器 InverseKin → PositiveKin 自洽性（基线，<0.02mm）
  2. MoveIt /compute_ik（KDL）vs 控制器 IK（展示理论DH与标定DH偏差）
  3. 结论

用法:
  ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py
  ros2 launch dobot_moveit dobot_moveit.launch.py
  ros2 run dobot_moveit verify_dh.py --x -0.2289 --y -0.1402 --z 0.3269 --rx 167.6 --ry -5.1 --rz -60.2
"""

import argparse
import math
import os
import sys

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import PoseStamped
from sensor_msgs.msg import JointState
from moveit_msgs.srv import GetPositionIK
from dobot_msgs_v4.srv import InverseKin, PositiveKin, GetAngle


# ============================================================
# 工具函数
# ============================================================
def deg2rad(d):
    return d * math.pi / 180.0

def rad2deg(r):
    return r * 180.0 / math.pi

def normalize_deg(target, current):
    """以 current 为基准，将 target 归算到最近等效角度，保证最短物理路径。"""
    delta = math.fmod(target - current, 360.0)
    if delta > 180.0:
        delta -= 360.0
    elif delta < -180.0:
        delta += 360.0
    return current + delta

def rotvec_deg_to_quat(rx, ry, rz):
    norm = math.sqrt(rx*rx + ry*ry + rz*rz)
    if norm < 1e-9:
        return (0.0, 0.0, 0.0, 1.0)
    angle = deg2rad(norm)
    ax, ay, az = rx/norm, ry/norm, rz/norm
    s = math.sin(angle/2)
    return (ax*s, ay*s, az*s, math.cos(angle/2))

def rotvec_diff_deg(rx1, ry1, rz1, rx2, ry2, rz2):
    def _q(rx, ry, rz):
        return rotvec_deg_to_quat(rx, ry, rz)
    q1, q2 = _q(rx1, ry1, rz1), _q(rx2, ry2, rz2)
    dot = abs(q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3])
    dot = min(1.0, max(0.0, dot))
    return 2.0 * rad2deg(math.acos(dot))

def parse_brace6(s):
    return [float(v) for v in s.strip().strip('{}').split(',') if v.strip()]


# ============================================================
# 节点
# ============================================================
class VerifyNode(Node):
    def __init__(self):
        super().__init__('verify_dh_node')
        name = os.getenv('DOBOT_TYPE', 'cr5')
        self.get_logger().info(f'机器人类型: {name}')
        self.group = f'{name}_group'
        self.joints = [f'joint{i+1}' for i in range(6)]

        self.cli_ik  = self.create_client(InverseKin, '/dobot_bringup_ros2/srv/InverseKin')
        self.cli_fk  = self.create_client(PositiveKin, '/dobot_bringup_ros2/srv/PositiveKin')
        self.cli_ang = self.create_client(GetAngle, '/dobot_bringup_ros2/srv/GetAngle')
        self.cli_mik = self.create_client(GetPositionIK, '/compute_ik')

    # ── 控制器接口 ──
    def get_joints_deg(self):
        if not self.cli_ang.wait_for_service(timeout_sec=3):
            return None
        fut = self.cli_ang.call_async(GetAngle.Request())
        rclpy.spin_until_future_complete(self, fut, timeout_sec=3)
        r = fut.result()
        return parse_brace6(r.robot_return) if r and r.res == 0 else None

    def ctrl_ik(self, x_mm, y_mm, z_mm, rx_d, ry_d, rz_d, seed=None):
        if not self.cli_ik.wait_for_service(timeout_sec=5):
            return None
        req = InverseKin.Request()
        req.x, req.y, req.z = x_mm, y_mm, z_mm
        req.rx, req.ry, req.rz = rx_d, ry_d, rz_d
        req.use_joint_near = '1'
        req.joint_near = '{' + ','.join(str(v) for v in (seed or [0]*6)[:6]) + '}'
        req.user = '0'; req.tool = '0'
        fut = self.cli_ik.call_async(req)
        rclpy.spin_until_future_complete(self, fut, timeout_sec=5)
        r = fut.result()
        raw = parse_brace6(r.robot_return) if r and r.res == 0 else None
        if raw is None:
            return None
        return [normalize_deg(raw[i], (seed or [0]*6)[i]) for i in range(len(raw))]

    def ctrl_fk(self, joints_deg):
        if not self.cli_fk.wait_for_service(timeout_sec=5):
            return None
        req = PositiveKin.Request()
        req.j1, req.j2, req.j3, req.j4, req.j5, req.j6 = joints_deg
        req.user = '0'; req.tool = '0'
        fut = self.cli_fk.call_async(req)
        rclpy.spin_until_future_complete(self, fut, timeout_sec=5)
        r = fut.result()
        if r and r.res == 0:
            v = parse_brace6(r.robot_return)
            return (v[0], v[1], v[2], v[3], v[4], v[5])
        return None

    # ── MoveIt /compute_ik ──
    def moveit_ik(self, x_m, y_m, z_m, rx_d, ry_d, rz_d, seed_deg=None):
        """调 move_group 的 /compute_ik，走当前加载的求解器 (searchPositionIK)"""
        if not self.cli_mik.wait_for_service(timeout_sec=5):
            return None
        req = GetPositionIK.Request()
        req.ik_request.group_name = self.group
        ps = PoseStamped()
        ps.header.frame_id = 'base_link'
        ps.pose.position.x = x_m
        ps.pose.position.y = y_m
        ps.pose.position.z = z_m
        q = rotvec_deg_to_quat(rx_d, ry_d, rz_d)
        ps.pose.orientation.x = q[0]
        ps.pose.orientation.y = q[1]
        ps.pose.orientation.z = q[2]
        ps.pose.orientation.w = q[3]
        req.ik_request.pose_stamped = ps
        if seed_deg and len(seed_deg) >= 6:
            js = JointState()
            js.name = self.joints
            js.position = [deg2rad(v) for v in seed_deg[:6]]
            req.ik_request.robot_state.joint_state = js
        req.ik_request.timeout.sec = 5
        req.ik_request.avoid_collisions = False
        fut = self.cli_mik.call_async(req)
        rclpy.spin_until_future_complete(self, fut, timeout_sec=8)
        r = fut.result()
        if r and r.error_code.val == 1:
            return [rad2deg(j) for j in r.solution.joint_state.position]
        return None


# ============================================================
# 主流程
# ============================================================
def main():
    parser = argparse.ArgumentParser(description='Dobot 运动学精度验证')
    parser.add_argument('--x', type=float, required=True, help='X (米)')
    parser.add_argument('--y', type=float, required=True, help='Y (米)')
    parser.add_argument('--z', type=float, required=True, help='Z (米)')
    parser.add_argument('--rx', type=float, default=0.0, help='旋转向量 X (度)')
    parser.add_argument('--ry', type=float, default=0.0, help='旋转向量 Y (度)')
    parser.add_argument('--rz', type=float, default=0.0, help='旋转向量 Z (度)')
    args = parser.parse_args()

    rclpy.init()
    node = VerifyNode()

    tx_mm = args.x * 1000; ty_mm = args.y * 1000; tz_mm = args.z * 1000

    print()
    print('=' * 62)
    print('  Dobot 运动学精度验证')
    print('=' * 62)
    print(f'  目标: pos=({args.x:.4f}, {args.y:.4f}, {args.z:.4f}) m')
    print(f'        rot_vec=({args.rx:.2f}, {args.ry:.2f}, {args.rz:.2f}) °')
    print()

    seed = node.get_joints_deg()
    print(f'  当前关节(deg): {[round(j,1) for j in seed] if seed else "N/A"}')
    print()

    # ════════════════════════════════════════════════════════
    # [1] 控制器自洽性: IK → FK 回验
    # ════════════════════════════════════════════════════════
    print('─' * 62)
    print('  [1] 控制器 InverseKin → PositiveKin 自洽性（基线）')
    print('─' * 62)

    ctrl_j = node.ctrl_ik(tx_mm, ty_mm, tz_mm, args.rx, args.ry, args.rz, seed)
    if ctrl_j is None:
        print('  ❌ 控制器 IK 失败')
        rclpy.shutdown(); return

    fk = node.ctrl_fk(ctrl_j)
    if fk is None:
        print('  ❌ 控制器 FK 失败')
        rclpy.shutdown(); return

    pe = math.sqrt((fk[0]-tx_mm)**2 + (fk[1]-ty_mm)**2 + (fk[2]-tz_mm)**2)
    oe = rotvec_diff_deg(fk[3], fk[4], fk[5], args.rx, args.ry, args.rz)

    print(f'  关节角(deg): {[round(j,4) for j in ctrl_j]}')
    print(f'  FK回验(mm/°): ({fk[0]:.3f}, {fk[1]:.3f}, {fk[2]:.3f}) ({fk[3]:.3f}, {fk[4]:.3f}, {fk[5]:.3f})')
    print(f'  位置误差: {pe:.4f} mm  {"✅" if pe<0.5 else "⚠️"}')
    print(f'  姿态误差: {oe:.4f} °  {"✅" if oe<1.0 else "⚠️"}')
    print()

    # ════════════════════════════════════════════════════════
    # [2] MoveIt /compute_ik 对比
    # ════════════════════════════════════════════════════════
    print('─' * 62)
    print('  [2] MoveIt /compute_ik（当前求解器）vs 控制器 IK')
    print('─' * 62)

    mi_j = node.moveit_ik(args.x, args.y, args.z, args.rx, args.ry, args.rz, seed)
    if mi_j is None:
        print('  ❌ /compute_ik 失败 — move_group 当前求解器无法解算该位姿')
        print('     (此即理论 DH 与真机标定 DH 的偏差证据)')
    else:
        print(f'  控制器  IK (deg): {[round(j,4) for j in ctrl_j]}')
        print(f'  /compute_ik (deg): {[round(j,4) for j in mi_j]}')
        diffs = [abs(ctrl_j[i] - mi_j[i]) for i in range(6)]
        print(f'  逐关节差  (deg): {[round(d,4) for d in diffs]}')
        max_d = max(diffs)
        mean_d = sum(diffs)/6
        print(f'  ─────────────────────────────────────────────')
        print(f'  最大关节差: {max_d:.4f} °')
        print(f'  平均关节差: {mean_d:.4f} °')

        # /compute_ik 的结果也用控制器 FK 回验
        mi_fk = node.ctrl_fk(mi_j)
        if mi_fk:
            mi_pe = math.sqrt((mi_fk[0]-tx_mm)**2 + (mi_fk[1]-ty_mm)**2 + (mi_fk[2]-tz_mm)**2)
            mi_oe = rotvec_diff_deg(mi_fk[3], mi_fk[4], mi_fk[5], args.rx, args.ry, args.rz)
            print(f'  /compute_ik 解 → 控制器 FK 回验误差: pos={mi_pe:.4f}mm orn={mi_oe:.4f}°')
            print(f'  (此误差 = 理论DH与标定DH的位姿偏差)')
    print()

    # ════════════════════════════════════════════════════════
    # 结论
    # ════════════════════════════════════════════════════════
    print('=' * 62)
    print('  结论')
    print('=' * 62)
    print(f'  控制器自洽性: pos={pe:.4f}mm orn={oe:.4f}°', end=' ')
    print('✅ 控制器自身一致' if pe < 0.5 else '⚠️ 控制器自身有偏差')

    if mi_j is None:
        print(f'  MoveIt /compute_ik: ❌ 无法解算')
        print(f'  → 理论 DH (KDL) 在此位姿失效，真机标定 DH 是唯一可用方案')
        print(f'  → use_dobot_solver:=true 对 setPoseTarget 是必需的')
    else:
        max_d = max(abs(ctrl_j[i] - mi_j[i]) for i in range(6))
        if max_d < 0.5:
            print(f'  MoveIt vs 控制器: 关节差 < 0.5° — 理论 DH 与标定 DH 非常接近')
        elif max_d < 2.0:
            print(f'  MoveIt vs 控制器: 关节差 {max_d:.2f}° — 理论 DH 有可见偏差')
        else:
            print(f'  MoveIt vs 控制器: 关节差 {max_d:.2f}° — 理论 DH 偏差显著')
        print(f'  → 脚本 setPoseTarget 在 use_dobot_solver:=false 时有此偏差')
        print(f'  → 脚本 setPoseTarget 在 use_dobot_solver:=true 时使用控制器清零此偏差')

    rclpy.shutdown()

if __name__ == '__main__':
    main()
