#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile
from sensor_msgs.msg import JointState


class JointStatesRelay(Node):

    def __init__(self):
        super().__init__('joint_states_relay')

        self.declare_parameter('publish_frequency', 50.0)
        self.declare_parameter('joint_names', ['joint1', 'joint2', 'joint3',
                                                'joint4', 'joint5', 'joint6'])

        self.publish_frequency = self.get_parameter('publish_frequency').value
        self.joint_names = self.get_parameter('joint_names').value

        self._latest_positions = [0.0] * len(self.joint_names)
        self._has_data = False

        qos = QoSProfile(depth=10)
        self.subscription = self.create_subscription(
            JointState, '/joint_states_robot', self.callback, qos
        )
        self.publisher = self.create_publisher(JointState, '/joint_states', qos)

        self.timer = self.create_timer(1.0 / self.publish_frequency, self._publish)

        self.get_logger().info(
            f'Joint States Relay started (freq={self.publish_frequency}Hz)'
        )

    def callback(self, msg):
        n = min(len(msg.position), len(self.joint_names))
        self._latest_positions = list(msg.position[:n])
        if len(self._latest_positions) < len(self.joint_names):
            self._latest_positions.extend([0.0] * (len(self.joint_names) - len(self._latest_positions)))
        self._has_data = True

    def _publish(self):
        joint_state = JointState()
        joint_state.header.stamp = self.get_clock().now().to_msg()
        joint_state.name = list(self.joint_names)
        joint_state.position = list(self._latest_positions)
        joint_state.velocity = [0.0] * len(self.joint_names)
        joint_state.effort = [0.0] * len(self.joint_names)
        self.publisher.publish(joint_state)


def main(args=None):
    rclpy.init(args=args)
    node = JointStatesRelay()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()