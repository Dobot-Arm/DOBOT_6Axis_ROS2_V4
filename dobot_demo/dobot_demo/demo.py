#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import rclpy                                                                      # ROS2 Python接口库
from rclpy.node   import Node                                                     # ROS2 节点类
from dobot_msgs_v4.srv import *   # 自定义的服务接口                                   
import time

class adderClient(Node):
    def __init__(self, name):
        super().__init__(name)                                                    # ROS2节点父类初始化
        self.EnableRobot_l = self.create_client(EnableRobot,'/dobot_bringup_ros2/srv/EnableRobot')
        self.MovJ_l = self.create_client(MovJ,'/dobot_bringup_ros2/srv/MovJ')
        self.SpeedFactor_l = self.create_client(SpeedFactor,'/dobot_bringup_ros2/srv/SpeedFactor')
        self.MovL_l = self.create_client(MovL,'/dobot_bringup_ros2/srv/MovL')
        self.DO_l = self.create_client(DO,'/dobot_bringup_ros2/srv/DO' )
        while not self.EnableRobot_l.wait_for_service(timeout_sec=1.0):                  # 循环等待服务器端成功启动
            self.get_logger().info('service not available, waiting again...') 
                    
    def initialization(self):  # 初始化：速度、坐标系、负载、工具偏心等
        response = self.EnableRobot_l.call_async(EnableRobot.Request())
        print(response)
        spe = SpeedFactor.Request()
        spe.ratio = 10
        response = self.SpeedFactor_l.call_async(spe)
        print(response)

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
            print(response)
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
            print(response)
        else:
            print("无该指令")

    def DO(self, index, status):  # IO 控制夹爪/气泵
        DO_V = DO.Request()
        DO_V.index = index
        DO_V.status = status
        response = self.DO_l.call_async(DO_V)
        print(response)


def main(args=None):
    rclpy.init(args=args)                                                         # ROS2 Python接口初始化
    node = adderClient("service_adder_client")                                    # 创建ROS2节点对象并进行初始化
    #node.send_request()                                                           # 发送服务请求
    node.point("MovJ", 50, -8, 0, 0, 0, 0)
    node.point("MovJ", 0, -8, 0, 0, 0, 0)  
    time.sleep(3)  
    node.destroy_node()                                                           # 销毁节点对象
    rclpy.shutdown()                                                              # 关闭ROS2 Python接口
