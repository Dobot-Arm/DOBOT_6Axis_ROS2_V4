#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
@author FTX
@date 2025 / 03 / 03
'''

import time
import rclpy
from rclpy.action import ActionServer
from rclpy.node import Node
from control_msgs.action import FollowJointTrajectory
from trajectory_msgs.msg import JointTrajectory, JointTrajectoryPoint
from dobot_msgs_v4.srv import *  # 自定义的服务接口
import os

class FollowJointTrajectoryServer(Node):

    def __init__(self):
        super().__init__('dobot_group_controller')
        name = os.getenv("DOBOT_TYPE")
        # 创建FollowJointTrajectory动作服务器
        self._action_server = ActionServer(self,FollowJointTrajectory,f'/{name}_group_controller/follow_joint_trajectory',self.execute_callback)
        self.get_logger().info("FollowJointTrajectory Action Server is ready...")
        self.EnableRobot_l = self.create_client(EnableRobot, '/dobot_bringup_ros2/srv/EnableRobot')
        self.ServoJ_l = self.create_client(ServoJ, '/dobot_bringup_ros2/srv/ServoJ')
        while not self.EnableRobot_l.wait_for_service(timeout_sec=1.0):  # 循环等待服务器端成功启动
            self.get_logger().info('service not available, waiting again...')

    async def execute_callback(self, goal_handle):
        self.get_logger().info("Received a new trajectory goal!")
        # 获取目标轨迹
        trajectory = goal_handle.request.trajectory
        self.execution_trajectory(trajectory)
        goal_handle.succeed()
        # 返回结果
        result = FollowJointTrajectory.Result()
        result.error_code = 0
        return result

    def execution_trajectory(self, trajectory: JointTrajectory):
        self.get_logger().info("Joint Names: {}".format(trajectory.joint_names))
        
        # 存储所有需要执行的点和时间戳
        all_points_with_time = []
        
        for i, point in enumerate(trajectory.points):
            joint = []
            for ii in point.positions:
                joint.append(180 * ii / 3.14159)  # 弧度转角度
            
            # 获取时间戳（秒）
            time_from_start = point.time_from_start.sec + point.time_from_start.nanosec / 1e9
            all_points_with_time.append((joint, time_from_start))
            
            self.get_logger().info(
                "Point {}: Positions: {}, TimeFromStart: {:.3f}s".format(
                    i, joint, time_from_start
                )
            )
        
        # 按照轨迹的时间戳执行
        self.execute_trajectory(all_points_with_time)

    def execute_trajectory(self, points_with_time):
        """按照轨迹时间戳执行轨迹点，动态设置ServoJ的t参数"""
        if not points_with_time:
            return
        
        self.get_logger().info(f"开始执行轨迹，共{len(points_with_time)}个点")
        
        # 记录开始时间
        start_time = time.time()
        
        for i, (joint_positions, target_time_from_start) in enumerate(points_with_time):
            # 计算当前点与前一个点的时间差作为t参数
            if i == 0:
                # 第一个点使用较小的初始时间
                t = 0.05
            else:
                prev_time = points_with_time[i-1][1]
                t = target_time_from_start - prev_time
            
            # 确保t在有效范围内 [0.004, 3600.0]
            t = max(0.004, min(t, 3600.0))
            
            # 等待到正确的时间
            current_time = time.time()
            elapsed_time = current_time - start_time
            
            if elapsed_time < target_time_from_start:
                sleep_time = target_time_from_start - elapsed_time
                if sleep_time > 0:
                    time.sleep(sleep_time)
            
            # 执行这个点（带动态计算的t参数）
            self.ServoJ_C_with_t(
                joint_positions[0], joint_positions[1], joint_positions[2],
                joint_positions[3], joint_positions[4], joint_positions[5],
                t
            )
            
            # 打印执行进度（每10个点或最后一个点）
            actual_time = time.time() - start_time
            if i % 10 == 0 or i == len(points_with_time) - 1:
                self.get_logger().info(f"点{i}: 计划时间={target_time_from_start:.3f}s, 实际时间={actual_time:.3f}s, t={t:.3f}s")

    def ServoJ_C_with_t(self, j1, j2, j3, j4, j5, j6, t):
        """带t参数的ServoJ命令"""
        P1 = ServoJ.Request()
        P1.a = float(j1)
        P1.b = float(j2)
        P1.c = float(j3)
        P1.d = float(j4)
        P1.e = float(j5)
        P1.f = float(j6)
        
        # 设置t参数（单位：秒）
        P1.param_value = [f"t={t}"]
        
        response = self.ServoJ_l.call_async(P1)

def main(args=None):
    rclpy.init(args=args)
    follow_joint_trajectory_server = FollowJointTrajectoryServer()
    rclpy.spin(follow_joint_trajectory_server)
    follow_joint_trajectory_server.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()