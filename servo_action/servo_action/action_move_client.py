#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#ros2 run servo_action action_move_client

import time

import rclpy                                    
from rclpy.node   import Node                    
from rclpy.action import ActionClient             
from control_msgs.action import FollowJointTrajectory  
import trajectory_msgs
import os

class MoveCircleActionClient(Node):
    def __init__(self, name):
        super().__init__(name)                   
        mane = os.getenv("DOBOT_TYPE")
        self._action_client = ActionClient(      
            self, FollowJointTrajectory, f'{mane}_group_controller/follow_joint_trajectory') 

    def send_goal(self, enable):                 
        joint = 0.1
        while True:
            if joint>2:
                joint = 0.1
            goal_msg = FollowJointTrajectory.Goal()            
            goal_msg .trajectory.joint_names =  ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
            goal_msg.trajectory.points.append(trajectory_msgs.msg.JointTrajectoryPoint(positions = [joint,0.4,joint,joint,0.2,0.4]))
            print(goal_msg.trajectory.points[0])
            self._action_client.wait_for_server()                          
            self._send_goal_future = self._action_client.send_goal_async(  
                goal_msg,                                                  
                feedback_callback=self.feedback_callback)                  
            joint = joint + 0.1
            time.sleep(1)
            
    def feedback_callback(self, feedback_msg):                               
        print(1)
        # time.sleep(5)
        #feedback = feedback_msg.feedback                                    
        #self.get_logger().info('Received feedback: {%d}' % feedback.state) 
def main(args=None):                                      
    rclpy.init(args=args)                                 
    node = MoveCircleActionClient("action_move_client")   
    node.send_goal(True)                                  
    rclpy.spin(node)                                      
    node.destroy_node()                                   
    rclpy.shutdown()                                      
