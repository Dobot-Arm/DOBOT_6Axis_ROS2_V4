<div align="center">

 <img src="image/dobot_moveit.jpg" alt="DOBOT 6Axis ROS2 V4" style="max-width: 600px; margin-bottom: 20px;" />

 <h1>DOBOT 6Axis ROS2 V4</h1>

 **越疆机器人 ROS2 软件开发套件**  
 基于 TCP/IP 协议的高性能机器人控制框架

 [English](README.md) · [简体中文](README_ZH.md)

 [![Platform](https://img.shields.io/badge/Platform-Ubuntu%2022.04-blue?style=flat-square)](https://ubuntu.com/download/server)
 [![ROS](https://img.shields.io/badge/ROS2-Humble-green?style=flat-square)](https://docs.ros.org/en/humble/)
 [![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

</div>

---

## 快速开始

### 环境要求

| 要求 | 版本 |
|------|------|
| 操作系统 | Ubuntu 22.04 LTS |
| ROS 版本 | ROS2 Humble |
| Python | 3.8+ |

### 网络配置

| 配置项 | 说明 |
|--------|------|
| 机器人 IP | 192.168.5.1（需与本机同一网段） |
| 控制端口 | 29999 |
| 反馈端口 | 30004 |

### 安装步骤

```bash
# 创建工作空间
mkdir -p ~/dobot_ws/src
cd ~/dobot_ws/src
git clone -b feature/v4-optimization https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
cd ~/dobot_ws

# 安装依赖
sudo apt update && sudo apt install -y \
  ros-humble-moveit \
  ros-humble-gazebo-* \
  ros-humble-joint-state-publisher \
  ros-humble-robot-state-publisher

# 编译
colcon build
source install/setup.bash

# 配置机械臂连接 IP（默认为有线连接 IP）
echo "export IP_address=192.168.5.1" >> ~/.bashrc

# 指定机械臂型号（根据实际型号选择）
# 示例：CR5 型号
echo "export DOBOT_TYPE=cr5" >> ~/.bashrc
# 支持型号：CR3、CR5、CR7、CR10、CR12、CR16、CR20、E6（ME6）、CR10AF、Nova2、Nova5、CR30H

# 生效配置
source ~/.bashrc
```

---

## 使用方式

### 1. RViz 可视化（独立模式）

不连接真实机器人的情况下查看模型：

```bash
ros2 launch dobot_rviz dobot_rviz.launch.py
```

### 2. RViz 连接真实机器人

将真实机器人的关节状态实时显示在 RViz 中：

```bash
# 终端1：连接机器人
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 终端2：启动 RViz 并启用实机模式
ros2 launch dobot_rviz dobot_rviz.launch.py live_hardware:=true
```

### 3. MoveIt 虚拟演示

在 RViz 中测试运动规划（无需连接机器人）：

```bash
ros2 launch dobot_moveit moveit_demo.launch.py
```

### 4. MoveIt 控制真实机器人

完整的运动规划与执行（需连接机器人）：

```bash
# 终端1：连接机器人
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 终端2：MoveIt 控制界面
ros2 launch dobot_moveit dobot_moveit.launch.py
```

### 5. Gazebo + MoveIt 联合仿真

物理仿真与运动规划联动：

```bash
# 终端1：启动 Gazebo 并加载 MoveIt 控制器
ros2 launch dobot_gazebo gazebo_moveit.launch.py

# 终端2：启动 MoveIt 控制界面
ros2 launch dobot_moveit moveit_gazebo.launch.py
```

### 6. 运动演示脚本

运行预构建的运动演示程序进行快速测试：

```bash
# 终端1：先连接机器人
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 终端2：运行基础运动演示（包含 MovJ/MovL/DO 控制）
ros2 run dobot_demo demo
```

### 7. 伺服动作客户端

通过动作客户端测试关节轨迹控制（周期性发送单步轨迹目标）：

```bash
# 终端1：连接机器人
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 终端2：启动 MoveIt 动作服务器
ros2 launch dobot_moveit dobot_moveit.launch.py

# 终端3：运行动作客户端（每秒钟发送一个单步目标）
ros2 run servo_action action_move_client
```

---

## 启动参数

以下参数适用于 `dobot_rviz.launch.py`：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `live_hardware` | `false` | 设为 `true` 时直接从真实机器人获取关节状态 |
| `gui` | `false` | 启用 `joint_state_publisher_gui` 手动控制关节 |
| `model` | 自动 | 机器人 URDF 文件路径（根据 `DOBOT_TYPE` 自动生成） |

### 环境变量配置

除了 launch 参数，项目还支持以下环境变量：

| 环境变量 | 默认值来源 | 说明 |
|----------|-----------|------|
| `DOBOT_TYPE` | `param.json` | 机器人型号（如 `cr5`、`cr10`、`nova2`） |
| `IP_address` | `param.json` | 机器人 IP 地址（用于真实机器人连接） |

---

## 项目结构

```
DOBOT_6Axis_ROS2_V4/
├── dobot_bringup_v4/        # 机器人驱动（TCP/IP 通信）
├── dobot_moveit/            # MoveIt 动作服务与工具
├── dobot_rviz/              # RViz 可视化启动器
├── dobot_gazebo/            # Gazebo 仿真
├── dobot_demo/              # 简单运动演示脚本
├── dobot_msgs_v4/           # ROS2 服务与消息定义
├── servo_action/            # 关节轨迹动作客户端
├── cr3_moveit/              # CR3 MoveIt 配置
├── cr5_moveit/              # CR5 MoveIt 配置
├── cr7_moveit/              # CR7 MoveIt 配置
├── cr10_moveit/             # CR10 MoveIt 配置
├── cr10af_moveit/           # CR10AF MoveIt 配置
├── cr12_moveit/             # CR12 MoveIt 配置
├── cr16_moveit/             # CR16 MoveIt 配置
├── cr20_moveit/             # CR20 MoveIt 配置
├── cr30h_moveit/            # CR30H MoveIt 配置
├── me6_moveit/              # E6/ME6 MoveIt 配置
├── nova2_moveit/            # Nova2 MoveIt 配置
├── nova5_moveit/            # Nova5 MoveIt 配置
├── cra_description/         # 机器人 URDF/XACRO 描述与网格文件
├── image/                   # 图片
├── README.md
└── README_ZH.md
```

---

## 架构与数据流

### MoveIt 控制真实机器人

```
dobot_bringup_v4 (main.cpp)
  └→ /joint_states_robot  (10Hz, 弧度, 直接从 30004 端口反馈)
       └→ joint_state_relay (--robot 模式)
            ├→ /joint_states           → MoveIt current_state_monitor (规划起点)
            └→ /rsp_joint_states       → robot_state_publisher  → TF  → RViz

MoveIt (OMPL / CHOMP 规划器)
  └→ FollowJointTrajectory Action
       └→ action_move_server (dobot_moveit 包)
            └→ ServoJ 服务 (逐个轨迹点)  → dobot_bringup_v4  → TCP  → 真实机器人
```

### 关键组件

| 组件 | 包 | 职责 |
|------|-----|------|
| `main.cpp` | `dobot_bringup_v4` | TCP 30004 端口实时反馈 → `/joint_states_robot` |
| `joint_state_relay.py` | `dobot_rviz` | 桥接 `/joint_states_robot` → `/joint_states`；中继 → `/rsp_joint_states` |
| `action_move_server.py` | `dobot_moveit` | 接收 MoveIt 轨迹，逐个点发送 ServoJ 指令给真实机器人 |
| `ompl_planning.yaml` | `{型号}_moveit` | OMPL 规划器配置（含时间参数化 `AddTimeOptimalParameterization`） |

---

## 支持型号

| 系列 | 型号 |
|------|------|
| CR 系列 | CR3、CR5、CR7、CR10、CR12、CR16、CR20、CR30H |
| CRAF 系列 | CR10AF |
| E 系列 | E6 / ME6 |
| Nova 系列 | Nova2、Nova5 |

---

## 启动文件参考

| 启动文件 | 说明 |
|----------|------|
| `dobot_rviz.launch.py` | RViz 可视化（独立模式或加 `live_hardware:=true`） |
| `dobot_bringup_ros2.launch.py` | 机器人驱动 — 与真实机器人建立 TCP/IP 连接 |
| `moveit_demo.launch.py` | MoveIt 运动规划演示（虚拟模式） |
| `dobot_moveit.launch.py` | MoveIt 控制界面（真实机器人） |
| `dobot_gazebo.launch.py` | Gazebo 物理仿真（不含控制器） |
| `gazebo_moveit.launch.py` | Gazebo + MoveIt 联合仿真（含控制器） |
| `dobot_joint.launch.py` | FollowJointTrajectory 动作服务器（用于真实机器人轨迹执行） |

## 演示程序与工具参考

| 包名 | 可执行文件 | 说明 |
|------|------------|------|
| `dobot_demo` | `demo` | 基础运动演示（包含 MovJ/MovL/DO 控制） |
| `servo_action` | `action_move_client` | 关节轨迹动作客户端（周期性发送目标） |
| `servo_action` | `Joint_Position` | 仿真测试用的虚拟关节状态发布器 |

---

## 注意事项

> ⚠️ **安全第一**：运行前确保机器人在安全位置

1. 确保电脑 IP 与机器人在同一网段（192.168.X.X）
2. 确保端口 29999 和 30004 未被占用
3. 机器人需处于远程 TCP/IP 控制模式

---

## 版本信息

| 信息 | 内容 |
|------|------|
| 当前版本 | V4.6.5 |
| ROS 版本 | ROS2 Humble |
| 协议版本 | Dobot TCP/IP V4.6.5 |

---

## 许可证

[MIT License](LICENSE)

<div align="center">
Built by Dobot-Arm
</div>
