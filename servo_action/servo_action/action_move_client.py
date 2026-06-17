#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# 关节轨迹动作客户端 - 周期性发送单步轨迹目标

import rclpy                                    
from rclpy.node   import Node                    
from rclpy.action import ActionClient             
from control_msgs.action import FollowJointTrajectory  
import trajectory_msgs
import os

class MoveCircleActionClient(Node):
    def __init__(self, name):
        super().__init__(name)                   
        name_env = os.getenv("DOBOT_TYPE", "cr5")
        self._action_client = ActionClient(      
            self, FollowJointTrajectory, f'/{name_env}_group_controller/follow_joint_trajectory') 
        self.joint = 0.1
        self._goal_active = False               # 跟踪是否有 Goal 正在执行
        self.timer = self.create_timer(1.0, self.timer_callback)
        self.get_logger().info("Action Move Client node started...")

    def timer_callback(self):                 
        # 上一个 Goal 还在执行中，跳过本次发送
        if self._goal_active:
            self.get_logger().info("Previous goal still active, skipping...")
            return

        if self.joint > 2.0:
            self.joint = 0.1
            
        goal_msg = FollowJointTrajectory.Goal()            
        goal_msg.trajectory.joint_names = ["joint1", "joint2", "joint3", "joint4", "joint5", "joint6"]
        goal_msg.trajectory.points.append(
            trajectory_msgs.msg.JointTrajectoryPoint(
                positions=[self.joint, 0.4, self.joint, self.joint, 0.2, 0.4]
            )
        )
        self.get_logger().info(f"Sending goal: joint={self.joint:.1f}")
        
        if self._action_client.wait_for_server(timeout_sec=1.0):                          
            self._goal_active = True
            self._send_goal_future = self._action_client.send_goal_async(  
                goal_msg,                                                  
                feedback_callback=self.feedback_callback)                  
            self._send_goal_future.add_done_callback(self.goal_response_callback)
        else:
            self.get_logger().warn("Action server not available")

    def goal_response_callback(self, future):
        """Goal 响应回调：检查是否被接受，若接受则等待结果"""
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().warn("Goal rejected by server")
            self._goal_active = False
            return
        self.get_logger().info("Goal accepted, waiting for result...")
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """结果回调：Goal 执行完毕，准备下一个"""
        result = future.result().result
        self.get_logger().info(f"Goal completed: error_code={result.error_code}")
        self._goal_active = False
        self.joint += 0.1
            
    def feedback_callback(self, feedback_msg):                               
        self.get_logger().info('收到反馈')

def main(args=None):                                      
    rclpy.init(args=args)                                 
    node = MoveCircleActionClient("action_move_client")   
    rclpy.spin(node)                                      
    node.destroy_node()                                   
    rclpy.shutdown()