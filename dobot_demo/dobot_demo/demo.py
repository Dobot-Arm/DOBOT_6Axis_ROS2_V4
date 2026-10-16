#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import rclpy
from rclpy.node import Node
from dobot_msgs_v4.srv import EnableRobot, MovJ, MovL, SpeedFactor, DO

class DobotDemo(Node):
    def __init__(self, name):
        super().__init__(name)
        self.EnableRobot_l = self.create_client(EnableRobot, '/dobot_bringup_ros2/srv/EnableRobot')
        self.MovJ_l        = self.create_client(MovJ,        '/dobot_bringup_ros2/srv/MovJ')
        self.SpeedFactor_l = self.create_client(SpeedFactor, '/dobot_bringup_ros2/srv/SpeedFactor')
        self.MovL_l        = self.create_client(MovL,        '/dobot_bringup_ros2/srv/MovL')
        self.DO_l          = self.create_client(DO,          '/dobot_bringup_ros2/srv/DO')

        # 等待所有必需的服务上线
        for cli, label in [(self.EnableRobot_l, 'EnableRobot'), (self.MovJ_l, 'MovJ')]:
            while not cli.wait_for_service(timeout_sec=1.0):
                self.get_logger().info(f'{label} service not available, waiting...')
        self.get_logger().info('All services available, ready.')

    def initialization(self):
        """使能机器人并设置速度比例"""
        # 使能
        response = self.EnableRobot_l.call_async(EnableRobot.Request())
        rclpy.spin_until_future_complete(self, response)
        result = response.result()
        if result is not None and result.res == 0:
            self.get_logger().info('EnableRobot OK')
        else:
            self.get_logger().error(f'EnableRobot failed: {result}')

        # 速度设为 10%
        spe = SpeedFactor.Request()
        spe.ratio = 10
        response = self.SpeedFactor_l.call_async(spe)
        rclpy.spin_until_future_complete(self, response)
        result = response.result()
        if result is not None and result.res == 0:
            self.get_logger().info(f'SpeedFactor set to {spe.ratio}%')
        else:
            self.get_logger().error(f'SpeedFactor failed: {result}')

    def move(self, Move, X_j1, Y_j2, Z_j3, RX_j4, RY_j5, RZ_j6):
        """执行运动指令 (MovJ 或 MovL)"""
        if Move == "MovJ":
            req = MovJ.Request()
            req.mode = True
            req.a = float(X_j1)
            req.b = float(Y_j2)
            req.c = float(Z_j3)
            req.d = float(RX_j4)
            req.e = float(RY_j5)
            req.f = float(RZ_j6)
            response = self.MovJ_l.call_async(req)
            rclpy.spin_until_future_complete(self, response)
            result = response.result()
            if result is not None and result.res == 0:
                self.get_logger().info(f'MovJ OK: [{X_j1}, {Y_j2}, {Z_j3}, {RX_j4}, {RY_j5}, {RZ_j6}]')
            else:
                self.get_logger().error(f'MovJ failed: {result}')
        elif Move == "MovL":
            req = MovL.Request()
            req.mode = True
            req.a = float(X_j1)
            req.b = float(Y_j2)
            req.c = float(Z_j3)
            req.d = float(RX_j4)
            req.e = float(RY_j5)
            req.f = float(RZ_j6)
            response = self.MovL_l.call_async(req)
            rclpy.spin_until_future_complete(self, response)
            result = response.result()
            if result is not None and result.res == 0:
                self.get_logger().info(f'MovL OK: [{X_j1}, {Y_j2}, {Z_j3}, {RX_j4}, {RY_j5}, {RZ_j6}]')
            else:
                self.get_logger().error(f'MovL failed: {result}')
        else:
            self.get_logger().warning(f"Unknown move type: {Move}")

    def set_do(self, index, status, time_ms=0):
        """控制数字输出 (夹爪/气泵)
        index:   IO 索引
        status:  0=OFF, 1=ON
        time_ms: 脉冲时间(ms), 0=永久保持
        """
        req = DO.Request()
        req.index = int(index)
        req.status = int(status)
        req.time = int(time_ms)
        response = self.DO_l.call_async(req)
        rclpy.spin_until_future_complete(self, response)
        result = response.result()
        if result is not None and result.res == 0:
            self.get_logger().info(f'DO {index} = {status} (time={time_ms}ms)')
        else:
            self.get_logger().error(f'DO {index} failed: {result}')


def main(args=None):
    rclpy.init(args=args)
    node = DobotDemo("dobot_demo")

    # 初始化：使能机器人 + 设置速度
    node.initialization()

    # 示例运动序列
    node.move("MovJ", 50, -8, 0, 0, 0, 0)
    node.move("MovJ",  0, -8, 0, 0, 0, 0)

    node.destroy_node()
    rclpy.shutdown()
