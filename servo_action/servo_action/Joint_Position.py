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
        # 注意：此节点为仿真测试用，发布话题为 servo_joint_states（非 joint_states），
        # 避免与 dobot_moveit/joint_states.py 的真实关节状态冲突
        self.pub2 = self.create_publisher(JointState, "servo_joint_states", 10)
        # 使用定时器定期发布关节状态（10Hz）
        self.timer = self.create_timer(0.1, self.timer_callback)
        # 初始关节位置
        self.joint_positions = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.get_logger().info("Servo fake joint_states node started, publishing servo_joint_states...")

    def timer_callback(self):
        msg2 = JointState()
        msg2.name = ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
        msg2.header.stamp = self.get_clock().now().to_msg()
        msg2.header.frame_id = ''  # 关节状态不需要参考坐标系
        msg2.position = self.joint_positions
        msg2.velocity = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        msg2.effort = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.pub2.publish(msg2)


def main(args=None):
    rclpy.init(args=args)
    node = PublisherNode("servo_fake_joint_states")
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()