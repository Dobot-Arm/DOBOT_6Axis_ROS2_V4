#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Fake demo server for MoveIt virtual demo.
- Publishes /joint_states with proper timestamps.
- Provides a fake FollowJointTrajectory action server on
  {robot_type}_group_controller/follow_joint_trajectory
  that simulates trajectory execution and updates joint positions.
"""
import threading
import rclpy
from rclpy.node import Node
from rclpy.action import ActionServer, GoalResponse
from rclpy.callback_groups import ReentrantCallbackGroup
from sensor_msgs.msg import JointState
from control_msgs.action import FollowJointTrajectory


class FakeDemoServer(Node):

    def __init__(self):
        super().__init__('fake_demo_server')

        self.declare_parameter('robot_type', 'cr5')
        self.declare_parameter('joint_names', ['joint1', 'joint2', 'joint3',
                                                'joint4', 'joint5', 'joint6'])
        self.declare_parameter('initial_positions', [0.0, 0.0, 0.0, 0.0, 0.0, 0.0])
        self.declare_parameter('publish_frequency', 50.0)

        robot_type = self.get_parameter('robot_type').value
        self._joint_names = self.get_parameter('joint_names').value
        self._freq = self.get_parameter('publish_frequency').value
        self._positions = list(self.get_parameter('initial_positions').value)

        # Publish /joint_states
        self._pub = self.create_publisher(JointState, '/joint_states', 10)
        self._pub_timer = self.create_timer(1.0 / self._freq, self._publish_joint_states)

        # Fake FollowJointTrajectory action server on the namespaced action
        # that moveit_simple_controller_manager connects to
        action_name = f'{robot_type}_group_controller/follow_joint_trajectory'
        self._action_server = ActionServer(
            self, FollowJointTrajectory, action_name,
            execute_callback=self._on_execute,
            goal_callback=self._on_goal,
            callback_group=ReentrantCallbackGroup(),
        )

        self._cancel_flag = threading.Event()

        self.get_logger().info(
            f'FakeDemoServer ready on /{action_name}'
        )

    # ── Joint states publisher ───────────────────────────
    def _publish_joint_states(self):
        msg = JointState()
        msg.header.stamp = self.get_clock().now().to_msg()
        msg.name = list(self._joint_names)
        msg.position = list(self._positions)
        msg.velocity = [0.0] * len(self._joint_names)
        msg.effort = [0.0] * len(self._joint_names)
        self._pub.publish(msg)

    # ── Action callbacks ─────────────────────────────────
    def _on_goal(self, goal_request):
        self.get_logger().info('Fake trajectory goal accepted')
        return GoalResponse.ACCEPT

    def _on_execute(self, goal_handle):
        self.get_logger().info('Executing fake trajectory...')
        self._cancel_flag.clear()

        result = FollowJointTrajectory.Result()
        traj = goal_handle.request.trajectory

        if not traj.points:
            goal_handle.abort()
            result.error_code = FollowJointTrajectory.Result.INVALID_GOAL
            return result

        # Simulate: jump to the final point
        last_point = traj.points[-1]
        n = min(len(last_point.positions), len(self._joint_names))
        for i in range(n):
            self._positions[i] = last_point.positions[i]
        self.get_logger().info(
            f'Trajectory done, position={[round(p, 3) for p in self._positions]}'
        )

        goal_handle.succeed()
        result.error_code = FollowJointTrajectory.Result.SUCCESSFUL
        return result


def main(args=None):
    rclpy.init(args=args)
    node = FakeDemoServer()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
