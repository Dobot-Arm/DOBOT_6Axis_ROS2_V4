#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ros2 run servo_action action_move_server

import time

import rclpy                                      # ROS2 Python接口库
from rclpy.node   import Node                     # ROS2 节点类
from rclpy.action import ActionServer             # ROS2 动作服务器类
from control_msgs.action import FollowJointTrajectory  
from moveit_msgs.msg import DisplayTrajectory                 
from trajectory_msgs.msg import JointTrajectoryPoint
import os


class MoveCircleActionServer(Node):
    def __init__(self, name):
        super().__init__(name)                   # ROS2节点父类初始化
        mane = os.getenv("DOBOT_TYPE")
        self._action_server = ActionServer(      # 创建动作服务器（接口类型、动作名、回调函数）
            self,
            FollowJointTrajectory,
            f'{mane}_group_controller/follow_joint_trajectory',
            self.execute_callback)
        self.sub = self.create_subscription(\
            DisplayTrajectory, "/display_planned_path", self.listener_callback, 10)        # 创建订阅者对象（消息类型、话题名、订阅者回调函数、队列长度）
    def listener_callback(self, msg): 
        self.adc = []                                              # 创建回调函数，执行收到话题消息后对数据的处理
        for i in msg.trajectory[0].joint_trajectory.points:        # 输出日志信息，提示订阅收到的话题消息
            acd = []
            for ii in i.positions:
                acd.append(180*ii/3.14159)
            self.adc.append(acd)
            

    def execute_callback(self, goal_handle):            # 执行收到动作目标之后的处理函数
        for i in self.adc:
          print(f"ServoJ:{i}")
        goal_handle.succeed()                           # 动作执行成功
        result = FollowJointTrajectory.Result()        
        result.error_code=0                           
        return result                                   # 反馈最终动作执行的结果

def main(args=None):                                       # ROS2节点主入口main函数
    rclpy.init(args=args)                                  # ROS2 Python接口初始化
    node = MoveCircleActionServer("action_move_server")    # 创建ROS2节点对象并进行初始化
    rclpy.spin(node)                                       # 循环等待ROS2退出
    node.destroy_node()                                    # 销毁节点对象
    rclpy.shutdown()                                       # 关闭ROS2 Python接口
