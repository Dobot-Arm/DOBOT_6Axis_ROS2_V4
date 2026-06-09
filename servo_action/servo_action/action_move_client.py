#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#ros2 run servo_action action_move_client

import rclpy                                    
from rclpy.node   import Node                    
from rclpy.action import ActionClient             
from control_msgs.action import FollowJointTrajectory  
import trajectory_msgs
import os

class MoveCircleActionClient(Node):
    def __init__(self, name):
        super().__init__(name)                   
        name_env = os.getenv("DOBOT_TYPE")
        self._action_client = ActionClient(      
            self, FollowJointTrajectory, f'/{name_env}_group_controller/follow_joint_trajectory') 
        self.joint = 0.1
        self.timer = self.create_timer(1.0, self.timer_callback)
        self.get_logger().info("Action Move Client node started...")

    def timer_callback(self):                 
        if self.joint > 2.0:
            self.joint = 0.1
            
        goal_msg = FollowJointTrajectory.Goal()            
        goal_msg.trajectory.joint_names = ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
        goal_msg.trajectory.points.append(
            trajectory_msgs.msg.JointTrajectoryPoint(
                positions=[self.joint, 0.4, self.joint, self.joint, 0.2, 0.4]
            )
        )
        self.get_logger().info(f"Sending goal: {goal_msg.trajectory.points[0]}")
        
        if self._action_client.wait_for_server(timeout_sec=1.0):                          
            self._send_goal_future = self._action_client.send_goal_async(  
                goal_msg,                                                  
                feedback_callback=self.feedback_callback)                  
        else:
            self.get_logger().warn("Action server not available")
            
        self.joint += 0.1
            
    def feedback_callback(self, feedback_msg):                               
        self.get_logger().info('收到反馈')

def main(args=None):                                      
    rclpy.init(args=args)                                 
    node = MoveCircleActionClient("action_move_client")   
    rclpy.spin(node)                                      
    node.destroy_node()                                   
    rclpy.shutdown()