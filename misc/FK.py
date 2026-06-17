#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import JointState
from moveit_msgs.srv import GetPositionFK
from moveit_msgs.msg import RobotState
from std_msgs.msg import Header
import math
import threading
import time
from datetime import datetime

class ContinuousFKCalculator(Node):
    def __init__(self, frequency=1.0):
        super().__init__('continuous_fk_calculator')
        
        # 订阅关节状态话题（真实硬件状态）
        self.joint_state_sub = self.create_subscription(
            JointState,
            '/joint_states_robot',
            self.joint_state_callback,
            10
        )
        
        # 服务客户端
        self.fk_client = self.create_client(GetPositionFK, '/compute_fk')
        
        # 存储状态
        self.joint_names = ['joint1', 'joint2', 'joint3', 'joint4', 'joint5', 'joint6']
        self.current_joint_state = None
        self.lock = threading.Lock()
        self.joint_data_available = threading.Event()
        
        # 控制参数
        self.frequency = frequency  # Hz
        self.calculation_interval = 1.0 / frequency
        self.min_state_change = 0.001  # 弧度
        
        # 标志位
        self.is_calculating = False
        self.service_available = False
        self.running = True
        
        # 统计信息
        self.calculation_count = 0
        self.success_count = 0
        self.failure_count = 0
        self.last_calculation_time = 0
        
        # 启动定时器检查服务
        self.create_timer(1.0, self.check_service_availability)
        
        # 启动计算线程
        self.calculation_thread = threading.Thread(target=self.calculation_loop)
        self.calculation_thread.start()
        
        print("\n" + "="*70)
        print("机械臂连续FK计算器")
        print("="*70)
        print(f"频率: {frequency} Hz (每 {self.calculation_interval:.2f} 秒计算一次)")
        print("模式: 固定频率计算，不依赖关节状态变化")
        print("="*70)
    
    def check_service_availability(self):
        """检查服务是否可用"""
        if not self.service_available and self.fk_client.wait_for_service(timeout_sec=1.0):
            self.service_available = True
            self.joint_data_available.set()  # 允许计算开始
            print("✅ FK服务已连接")
            print("等待关节数据...")
    
    def joint_state_callback(self, msg):
        """处理接收到的关节状态消息"""
        with self.lock:
            # 提取关节数据
            joint_angles = []
            valid_data = True
            
            for joint_name in self.joint_names:
                try:
                    idx = msg.name.index(joint_name)
                    if idx < len(msg.position):
                        joint_angles.append(msg.position[idx])
                    else:
                        valid_data = False
                        break
                except (ValueError, IndexError):
                    valid_data = False
                    break
            
            if valid_data and len(joint_angles) == len(self.joint_names):
                self.current_joint_state = joint_angles
                self.joint_data_available.set()  # 设置事件，表示有数据可用
    
    def calculation_loop(self):
        """计算循环，以固定频率计算FK"""
        while self.running:
            # 等待关节数据可用
            if not self.joint_data_available.wait(timeout=1.0):
                if self.running and self.service_available:
                    current_time = time.time()
                    if current_time - self.last_calculation_time > 5.0:  # 每5秒提醒一次
                        print(f"[{datetime.now().strftime('%H:%M:%S')}] 等待关节数据...")
                        self.last_calculation_time = current_time
                continue
            
            # 检查是否可以计算
            if (not self.service_available or self.is_calculating or 
                self.current_joint_state is None):
                time.sleep(0.1)  # 短暂等待
                continue
            
            # 获取当前时间
            calculation_start = time.time()
            
            # 设置计算标志
            self.is_calculating = True
            self.calculation_count += 1
            
            # 获取关节状态
            with self.lock:
                joint_angles = self.current_joint_state.copy() if self.current_joint_state else None
            
            if joint_angles is None:
                self.is_calculating = False
                continue
            
            # 计算FK
            try:
                pose = self.compute_fk_sync(joint_angles)
                
                if pose is not None:
                    self.success_count += 1
                    
                    # 显示结果
                    calculation_time = time.time() - calculation_start
                    self.display_result(pose, joint_angles, calculation_time)
                else:
                    self.failure_count += 1
                    
            except Exception as e:
                self.failure_count += 1
                print(f"[{datetime.now().strftime('%H:%M:%S')}] ❌ 计算异常: {e}")
            
            finally:
                # 重置计算标志
                self.is_calculating = False
                
                # 显示统计信息
                if self.calculation_count % 10 == 0:  # 每10次计算显示一次统计
                    self.display_statistics()
                
                # 等待到下一个计算周期
                elapsed = time.time() - calculation_start
                if elapsed < self.calculation_interval:
                    time.sleep(self.calculation_interval - elapsed)
    
    def compute_fk_sync(self, joint_positions):
        """同步计算正向运动学"""
        if not self.service_available or self.fk_client is None:
            return None
        
        # 准备请求
        request = GetPositionFK.Request()
        request.header = Header(frame_id='base_link', stamp=self.get_clock().now().to_msg())
        request.fk_link_names = ['Link6']
        
        joint_state = JointState()
        joint_state.name = self.joint_names
        joint_state.position = joint_positions
        
        robot_state = RobotState()
        robot_state.joint_state = joint_state
        robot_state.is_diff = False
        
        request.robot_state = robot_state
        
        # 调用服务
        future = self.fk_client.call_async(request)
        
        # 等待结果
        rclpy.spin_until_future_complete(self, future, timeout_sec=5.0)
        
        if future.result() is not None:
            response = future.result()
            if response.error_code.val == 1 and len(response.pose_stamped) > 0:
                return response.pose_stamped[0].pose
        
        return None
    
    def display_result(self, pose, joint_angles, calculation_time):
        """显示计算结果"""
        timestamp = datetime.now().strftime('%H:%M:%S')
        
        # 提取位置和姿态
        pos = pose.position
        orient = pose.orientation
        
        # 转换为欧拉角
        roll, pitch, yaw = self.quaternion_to_euler(orient.x, orient.y, orient.z, orient.w)
        
        # 显示计算结果
        print(f"\n[{timestamp}] 计算 #{self.calculation_count} (耗时: {calculation_time*1000:.1f}ms)")
        print("-" * 60)
        
        # 显示关节角度
        print("关节角度 (弧度/度):")
        for i, (name, angle) in enumerate(zip(self.joint_names, joint_angles)):
            angle_deg = math.degrees(angle)
            print(f"  {name}: {angle:8.6f} ({angle_deg:7.2f}°)")
        
        # 显示位姿
        print(f"\n位置:")
        print(f"  x = {pos.x:10.6f} m")
        print(f"  y = {pos.y:10.6f} m")
        print(f"  z = {pos.z:10.6f} m")
        
        print(f"\n姿态 (欧拉角):")
        print(f"  R = {math.degrees(roll):7.2f}°")
        print(f"  P = {math.degrees(pitch):7.2f}°")
        print(f"  Y = {math.degrees(yaw):7.2f}°")
        
        # 简单验证
        expected = self.get_expected_position(joint_angles)
        if expected:
            dx, dy, dz = expected
            error = math.sqrt((pos.x - dx)**2 + (pos.y - dy)**2 + (pos.z - dz)**2)
            if error > 0.001:
                print(f"  ⚠️ 位置误差: {error*1000:.2f} mm")
    
    def get_expected_position(self, joint_angles):
        """获取预期位置（用于验证）"""
        # 已知的关节角度和位置对应关系
        known_configs = {
            (0.0, 0.0, 0.0, 0.0, 0.0, 0.0): (1.8465e-06, -0.2333, 0.7548),
            (-0.613134, -0.410763, 1.731611, 0.068835, -1.440735, -3.547466): 
            (-0.283098, 0.025662, 0.361385),
            (-0.378894, -0.401478, 1.936495, -0.170676, -1.486043, -3.316702):
            (-0.307291, 0.033513, 0.287784)
        }
        
        # 查找最接近的配置
        for known_joints, position in known_configs.items():
            match = True
            for a, b in zip(joint_angles, known_joints):
                if abs(a - b) > 0.0001:
                    match = False
                    break
            if match:
                return position
        
        return None
    
    def display_statistics(self):
        """显示统计信息"""
        if self.calculation_count == 0:
            return
        
        success_rate = (self.success_count / self.calculation_count) * 100
        timestamp = datetime.now().strftime('%H:%M:%S')
        
        print(f"\n[{timestamp}] 📊 统计信息:")
        print("-" * 50)
        print(f"  总计算次数: {self.calculation_count}")
        print(f"  成功次数:   {self.success_count}")
        print(f"  失败次数:   {self.failure_count}")
        print(f"  成功率:     {success_rate:.1f}%")
        print(f"  计算频率:   {self.frequency} Hz")
        print("-" * 50)
    
    def quaternion_to_euler(self, x, y, z, w):
        """将四元数转换为欧拉角（roll, pitch, yaw）"""
        # roll (x-axis rotation)
        sinr_cosp = 2 * (w * x + y * z)
        cosr_cosp = 1 - 2 * (x * x + y * y)
        roll = math.atan2(sinr_cosp, cosr_cosp)
        
        # pitch (y-axis rotation)
        sinp = 2 * (w * y - z * x)
        if abs(sinp) >= 1:
            pitch = math.copysign(math.pi / 2, sinp)
        else:
            pitch = math.asin(sinp)
        
        # yaw (z-axis rotation)
        siny_cosp = 2 * (w * z + x * y)
        cosy_cosp = 1 - 2 * (y * y + z * z)
        yaw = math.atan2(siny_cosp, cosy_cosp)
        
        return roll, pitch, yaw
    
    def destroy_node(self):
        """销毁节点"""
        self.running = False
        self.joint_data_available.set()  # 唤醒线程
        
        if hasattr(self, 'calculation_thread') and self.calculation_thread.is_alive():
            self.calculation_thread.join(timeout=2.0)
        
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    
    # 创建节点
    node = ContinuousFKCalculator(frequency=1.0)  # 1 Hz
    
    print("\n📢 说明:")
    print("1. 脚本会以固定频率计算末端位姿，即使关节状态不变")
    print("2. 计算频率可通过构造函数参数调整")
    print("3. 每次计算会显示关节角度、末端位置和姿态")
    print("4. 每10次计算显示一次统计信息")
    print("="*70)
    print("按 Ctrl+C 退出")
    print("="*70)
    
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        print("\n\n🛑 程序被用户中断")
    except Exception as e:
        print(f"\n\n❌ 程序出错: {e}")
    finally:
        try:
            node.destroy_node()
        except:
            pass
        try:
            rclpy.shutdown()
        except:
            pass
        
        # 显示最终统计
        print("\n" + "="*70)
        print("最终统计:")
        node.display_statistics()
        print("="*70)


if __name__ == '__main__':
    main()
