#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
@author FTX
@date 2025 / 03 / 03
'''

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
import numpy as np


class PublisherNode(Node):

    def __init__(self, name):
        super().__init__(name)
        self.pub2 = self.create_publisher(JointState, "joint_states", 10)
        self.listener_callback()

    def listener_callback(self):
        msg2 = JointState()
        while True:
            msg2.name = ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
            msg2.header.stamp = self.get_clock().now().to_msg()
            msg2.header.frame_id = 'joint_states'
            msg2.position = joint
            self.pub2.publish(msg2)


def main(args=None):
    rclpy.init(args=args)
    node = PublisherNode("dobot_joint_states")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()
