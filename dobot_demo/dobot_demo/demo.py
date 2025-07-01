#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import rclpy
from rclpy.node   import Node
from dobot_msgs_v4.srv import *
import time

class adderClient(Node):
    def __init__(self, name):
        super().__init__(name)
        self.EnableRobot_l = self.create_client(EnableRobot,'/dobot_bringup_ros2/srv/EnableRobot')
        self.MovJ_l = self.create_client(MovJ,'/dobot_bringup_ros2/srv/MovJ')
        self.SpeedFactor_l = self.create_client(SpeedFactor,'/dobot_bringup_ros2/srv/SpeedFactor')
        self.MovL_l = self.create_client(MovL,'/dobot_bringup_ros2/srv/MovL')
        self.DO_l = self.create_client(DO,'/dobot_bringup_ros2/srv/DO' )
        while not self.EnableRobot_l.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('service not available, waiting again...') 
                    
    def initialization(self):  # 初始化：速度、坐标系、负载、工具偏心等
        response = self.EnableRobot_l.call_async(EnableRobot.Request())
        self.spin_until_future_complete(response)  # 等待响应
        self.get_logger().info(f"{response.result()}")
        spe = SpeedFactor.Request()
        spe.ratio = 10
        response = self.SpeedFactor_l.call_async(spe)
        self.spin_until_future_complete(response) 
        self.get_logger().info(f"{response.result()}")

    def point(self, Move, X_j1, Y_j2, Z_j3, RX_j4, RY_j5, RZ_j6):  # 运动指令
        if Move == "MovJ":
            P1 = MovJ.Request()
            P1.mode = True
            P1.a = float(X_j1)
            P1.b = float(Y_j2)
            P1.c = float(Z_j3)
            P1.d = float(RX_j4)
            P1.e = float(RY_j5)
            P1.f = float(RZ_j6)
            response = self.MovJ_l.call_async(P1)
            self.spin_until_future_complete(response)  
            self.get_logger().info(f"{response.result()}")
        elif Move == "MovL":
            P1 = MovL.Request()
            P1.mode = True
            P1.a = float(X_j1)
            P1.b = float(Y_j2)
            P1.c = float(Z_j3)
            P1.d = float(RX_j4)
            P1.e = float(RY_j5)
            P1.f = float(RZ_j6)
            response = self.MovL_l.call_async(P1)
            self.spin_until_future_complete(response) 
            self.get_logger().info(f"{response.result()}")
        else:
            print("无该指令")

    def DO(self, index, status):  # IO 控制夹爪/气泵
        DO_V = DO.Request()
        DO_V.index = index
        DO_V.status = status
        response = self.DO_l.call_async(DO_V)
        self.spin_until_future_complete(response)  # 等待响应
        self.get_logger().info(f"{response.result()}")


def main(args=None):
    rclpy.init(args=args)
    node = adderClient("dobot_demo")
    #node.send_request()
    node.point("MovJ", 50, -8, 0, 0, 0, 0)
    node.point("MovJ", 0, -8, 0, 0, 0, 0)  
    time.sleep(3)  
    node.destroy_node()
    rclpy.shutdown()
