#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#ros2 run servo_action action_move_client

import time

import rclpy                                      # ROS2 Python接口库
from rclpy.node   import Node                     # ROS2 节点类
from rclpy.action import ActionClient             
from control_msgs.action import FollowJointTrajectory  
import trajectory_msgs
import os

class MoveCircleActionClient(Node):
    def __init__(self, name):
        super().__init__(name)                   # ROS2节点父类初始化
        mane = os.getenv("DOBOT_TYPE")
        self._action_client = ActionClient(      # 创建动作客户端（接口类型、动作名）
            self, FollowJointTrajectory, f'{mane}_group_controller/follow_joint_trajectory') 

    def send_goal(self, enable):                 # 创建一个发送动作目标的函数
        aa = 0.1
        while True:
            if aa>2:
                aa = 0.1
            goal_msg = FollowJointTrajectory.Goal()             # 创建一个动作目标的消息
            goal_msg .trajectory.joint_names =  ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
            goal_msg.trajectory.points.append(trajectory_msgs.msg.JointTrajectoryPoint(positions = [aa,0.4,aa,aa,0.2,0.4]))
            print(goal_msg.trajectory.points[0])
            self._action_client.wait_for_server()                           # 等待动作的服务器端启动
            self._send_goal_future = self._action_client.send_goal_async(   # 异步方式发送动作的目标
                goal_msg,                                                   # 动作目标
                feedback_callback=self.feedback_callback)                   # 处理周期反馈消息的回调函数
            aa = aa + 0.1
            time.sleep(1)
            
    def feedback_callback(self, feedback_msg):                                # 创建处理周期反馈消息的回调函数
        print(1)
        # time.sleep(5)
        #feedback = feedback_msg.feedback                                      # 读取反馈的数据
        #self.get_logger().info('Received feedback: {%d}' % feedback.state) 
def main(args=None):                                       # ROS2节点主入口main函数
    rclpy.init(args=args)                                  # ROS2 Python接口初始化
    node = MoveCircleActionClient("action_move_client")    # 创建ROS2节点对象并进行初始化
    node.send_goal(True)                                   # 发送动作目标
    rclpy.spin(node)                                       # 循环等待ROS2退出
    node.destroy_node()                                    # 销毁节点对象
    rclpy.shutdown()                                       # 关闭ROS2 Python接口
