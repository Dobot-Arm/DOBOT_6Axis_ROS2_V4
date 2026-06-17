#!/usr/bin/env python3
"""Joint state relay for dobot_rviz.

Two modes:
  default   Sub /joint_states → pub /rsp_joint_states (for RSP isolation)
  --robot   Sub /joint_states_robot → pub /rsp_joint_states AND /joint_states
            (bridge: feeds both MoveIt and RSP from raw bringup data)

robot_state_publisher reads from /rsp_joint_states (isolated from ghost publishers).
"""
import sys
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from xml.dom import minidom

class JointStateRelay(Node):
    def __init__(self, urdf_path, bridge=False):
        super().__init__('joint_state_relay')
        self.bridge = bridge
        self.joint_names = self._parse_joints(urdf_path)
        self.get_logger().info(
            f'Mode: {"BRIDGE" if bridge else "RELAY"}, joints: {self.joint_names}')

        self.last_msg = None
        self.last_msg_time = 0

        if bridge:
            self.sub = self.create_subscription(
                JointState, 'joint_states_robot', self._on_joint_state, 10)
            self.bridge_pub = self.create_publisher(JointState, 'joint_states', 10)
        else:
            self.sub = self.create_subscription(
                JointState, 'joint_states', self._on_joint_state, 10)

        self.pub = self.create_publisher(JointState, 'rsp_joint_states', 10)
        self.timer = self.create_timer(0.1, self._publish)

    def _parse_joints(self, path):
        with open(path, 'r') as f:
            dom = minidom.parseString(f.read())
        joints = []
        for el in dom.getElementsByTagName('joint'):
            jtype = el.getAttribute('type')
            if jtype in ('fixed', 'floating', 'planar'):
                continue
            joints.append(el.getAttribute('name'))
        return joints

    def _on_joint_state(self, msg):
        if len(msg.name) == 0 or len(msg.position) != len(msg.name):
            return
        self.last_msg = msg
        self.last_msg_time = self.get_clock().now().nanoseconds

        # Bridge mode: also publish to /joint_states for MoveIt
        if self.bridge:
            out = JointState()
            out.header.stamp = msg.header.stamp
            out.name = list(self.joint_names)
            out.position = list(msg.position[:len(self.joint_names)]) if len(msg.position) >= len(self.joint_names) else [0.0] * len(self.joint_names)
            self.bridge_pub.publish(out)

    def _publish(self):
        now_ns = self.get_clock().now().nanoseconds

        if self.last_msg is not None and (now_ns - self.last_msg_time) < 1_000_000_000:
            out = JointState()
            out.header = self.last_msg.header
            out.name = list(self.last_msg.name)
            out.position = list(self.last_msg.position)
            out.velocity = list(self.last_msg.velocity) if self.last_msg.velocity else []
            out.effort = list(self.last_msg.effort) if self.last_msg.effort else []
            self.pub.publish(out)
        else:
            n = len(self.joint_names)
            msg = JointState()
            msg.header.stamp = self.get_clock().now().to_msg()
            msg.header.frame_id = 'dummy_link'
            msg.name = list(self.joint_names)
            msg.position = [0.0] * n
            self.pub.publish(msg)


def main():
    rclpy.init()
    if len(sys.argv) < 2:
        print('Usage: joint_state_relay <urdf_path> [--robot]')
        sys.exit(1)
    bridge = '--robot' in sys.argv
    node = JointStateRelay(sys.argv[1], bridge)
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
