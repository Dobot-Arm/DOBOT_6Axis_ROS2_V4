#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#ros2 run servo_action action_move_client_reality

import rclpy                                  
from rclpy.node   import Node                  
from sensor_msgs.msg import JointState   
from rclpy.action import ActionClient                         
from control_msgs.action import FollowJointTrajectory  
import trajectory_msgs
import os

class SubscriberNode(Node):
    def __init__(self, name):
        super().__init__(name)                                    # ROS2节点父类初始化
        mane = os.getenv("DOBOT_TYPE")
        self.sub = self.create_subscription(\
            JointState, "/joint_states_robot", self.listener_callback, 10)        # 创建订阅者对象（消息类型、话题名、订阅者回调函    数、队列长度）
        self._action_client = ActionClient(      # 创建动作客户端（接口类型、动作名）
            self, FollowJointTrajectory, f'{mane}_group_controller/follow_joint_trajectory')
    def listener_callback(self, msg):                             # 创建回调函数，执行收到话题消息后对数据的处理
        joint = msg.position[0:6]
        Joint_i = []
        for i in joint:
            Joint_i.append(round(i, 3))
        self.send_goal(Joint_i)
    def send_goal(self, joint):                 # 创建一个发送动作目标的函数
        print(joint)
        goal_msg = FollowJointTrajectory.Goal()             # 创建一个动作目标的消息
        goal_msg .trajectory.joint_names =  ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
        goal_msg.trajectory.points.append(trajectory_msgs.msg.JointTrajectoryPoint(positions = [joint[0],joint[1],joint[2],joint[3],joint[4],joint[5]]))
        print(goal_msg.trajectory.points[0])
        self._action_client.wait_for_server()                           # 等待动作的服务器端启动
        self._send_goal_future = self._action_client.send_goal_async(goal_msg,feedback_callback=self.feedback_callback)
    def feedback_callback(self, feedback_msg):                                # 创建处理周期反馈消息的回调函数
        print("OK")                   
        
             
        
def main(args=None):                                 # ROS2节点主入口main函数
    rclpy.init(args=args)                            # ROS2 Python接口初始化
    node = SubscriberNode("topic_helloworld_sub")    # 创建ROS2节点对象并进行初始化
    rclpy.spin(node)                                 # 循环等待ROS2退出
    node.destroy_node()                              # 销毁节点对象
    rclpy.shutdown()                                 # 关闭ROS2 Python接口

