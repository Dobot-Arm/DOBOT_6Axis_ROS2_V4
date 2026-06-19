#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
@author FTX
@date 2025 / 03 / 03
'''

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState

class PublisherNode(Node):
    
    def __init__(self, name):
        super().__init__(name)
        self.sub = self.create_subscription(JointState, "/joint_states_robot", self.listener_callback, 10)
        self.pub2 = self.create_publisher(JointState, "joint_states", 10)
        self.get_logger().info("节点已启动，等待数据...")

    def listener_callback(self, msg):
        try:
            joint = msg.position[0:6]
            msg2 = JointState()
            msg2.name = ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
            
            msg2.header.stamp = self.get_clock().now().to_msg()
            msg2.header.frame_id = ''
            msg2.position = joint
            msg2.velocity = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            msg2.effort = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
            self.pub2.publish(msg2)
            
            self.get_logger().debug(f"关节位置: {joint}")
            
        except Exception as e:
            self.get_logger().error(f"处理关节状态失败: {e}")

def main(args=None):
    rclpy.init(args=args)
    node = PublisherNode("dobot_joint_states")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()