#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
import rclpy                                  
from rclpy.node import Node                  
from sensor_msgs.msg import JointState            

"""
创建一个订阅者节点
"""
class SubscriberNode(Node):
    
    def __init__(self, name):
        super().__init__(name)                                    
        self.sub = self.create_subscription(\
            JointState, "/joint_states_robot", self.listener_callback, 10)       
    def listener_callback(self, msg):                            
        joint = msg.position[0:5]
        Joint_i = []
        for i in joint:
            Joint_i.append(round(i*57.2957, 3)) 
        print(Joint_i)
              
        
def main(args=None):                                 
    rclpy.init(args=args)                            
    node = SubscriberNode("topic_helloworld_sub")   
    rclpy.spin(node)                                
    node.destroy_node()                            
    rclpy.shutdown()                                 