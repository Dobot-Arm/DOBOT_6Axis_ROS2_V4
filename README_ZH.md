<div align="center">

<img src="image/dobot_moveit.jpg" alt="DOBOT 6Axis ROS2 V4" style="max-width: 600px; margin-bottom: 20px;" />

<h1>DOBOT 6Axis ROS2 V4</h1>

 **越疆机器人 ROS2 软件开发套件**
 基于 TCP/IP 协议的高性能机器人控制框架

 [English](README.md) · [简体中文](README_ZH.md)

 [![Platform](https://img.shields.io/badge/Platform-Ubuntu%2024.04-blue?style=flat-square)](https://ubuntu.com/download/server)
 [![ROS](https://img.shields.io/badge/ROS2-Jazzy-green?style=flat-square)](https://docs.ros.org/en/jazzy/)
 [![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

</div>

---

## 快速开始

### 环境要求

| 要求     | 版本             |
| -------- | ---------------- |
| 操作系统 | Ubuntu 24.04 LTS |
| ROS 版本 | ROS2 Jazzy       |
| Python   | 3.10+            |
| 仿真引擎 | Gazebo Sim (Harmonic) |

### 网络配置

| 配置项    | 说明                            |
| --------- | ------------------------------- |
| 机器人 IP | 192.168.5.1（需与本机同一网段） |
| 控制端口  | 29999                           |
| 反馈端口  | 30004                           |

### 安装步骤

```bash
# 创建工作空间
mkdir -p ~/dobot_ws/src
cd ~/dobot_ws/src
git clone -b jazzy https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
cd ~/dobot_ws

# 安装依赖
sudo apt update && sudo apt install -y \
  ros-jazzy-moveit \
  ros-jazzy-ros-gz-sim \
  ros-jazzy-ros-gz-bridge \
  ros-jazzy-gz-ros2-control \
  ros-jazzy-joint-state-publisher \
  ros-jazzy-robot-state-publisher

# 编译
colcon build --symlink-install
source install/local_setup.sh
```

> 详细安装指南请见 [docs/INSTALL_JAZZY.md](docs/INSTALL_JAZZY.md)。

---

## 机型切换与 IP 配置

### 切换机器人型号

项目通过环境变量 `DOBOT_TYPE` 来选择不同的机器人型号，支持的型号包括：

| DOBOT_TYPE  | 对应机器人型号 |
| ----------- | -------------- |
| `cr3`       | CR3            |
| `cr5`       | CR5（默认）    |
| `cr7`       | CR7            |
| `cr10`      | CR10           |
| `cr10af`    | CR10AF         |
| `cr12`      | CR12           |
| `cr16`      | CR16           |
| `cr20`      | CR20           |
| `cr30h`     | CR30H          |
| `me6`       | E6 / ME6       |
| `nova2`     | Nova2          |
| `nova5`     | Nova5          |

在终端中设置环境变量（以 CR5 为例）：

```bash
export DOBOT_TYPE=cr5
```

如需永久设置，可将上述命令添加到 `~/.bashrc` 文件中。

### 配置机器人 IP 地址

项目通过环境变量 `IP_address` 来传入机器人的实际 IP 地址：

```bash
export IP_address=192.168.5.1
```

> 默认机器人 IP 为 `192.168.5.1`，请根据实际网络情况修改。确保电脑与机器人在同一网段。

## 快速启动（5 种场景）

选择你需要的场景，执行前先设置 `DOBOT_TYPE`：

```bash
export DOBOT_TYPE=cr5
```

| # | 场景 | 命令 |
|---|------|------|
| 1 | RViz 可视化（仅查看模型） | `ros2 launch dobot_rviz dobot_rviz.launch.py` |
| 2 | MoveIt 虚拟仿真（无 Gazebo，无硬件） | `ros2 launch dobot_moveit moveit_demo.launch.py` |
| 3 | 单独 Gazebo 仿真（无 MoveIt） | `ros2 launch dobot_gazebo dobot_gazebo.launch.py` |
| 4 | **Gazebo + MoveIt** 联动仿真 | `ros2 launch dobot_gazebo sim.launch.py` |
| 5 | **MoveIt + 真机**（需硬件连接） | 见下方 |

> **场景 4（Gazebo + MoveIt 联动）：** `sim.launch.py` 一键启动 Gazebo、MoveGroup 和 RViz。
> 也可分开两个终端启动：
> ```bash
> # 终端 1
> ros2 launch dobot_gazebo dobot_gazebo.launch.py
> # 终端 2
> ros2 launch dobot_moveit moveit_gazebo.launch.py
> ```

### 场景 5：MoveIt + 真机

```bash
# 1. 设置机器人型号和 IP
export DOBOT_TYPE=cr5
export IP_address=192.168.5.1

# 2. 启动机器人驱动（终端 1）
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 3. 启动 MoveIt 控制界面（终端 2）
ros2 launch dobot_moveit dobot_moveit.launch.py
```

### 可选：运动演示

场景 5 启动后，可运行示例运动序列（EnableRobot + MovJ）：

```bash
ros2 run dobot_demo demo
```

---

## 系统架构

### 数据流（真机模式）

```
物理机器人 (TCP/IP)
  ├─ 192.168.5.1:29999  ← 控制指令（dashboard）
  └─ 192.168.5.1:30004  → 1440 字节二进制 RealTimeData @100Hz（反馈）

dobot_bringup_v4 (C++ 节点 "dobot_bringup_ros2")
  ├─ CRCommanderRos2: 持有两个 TcpClient，recvTask 线程读取反馈
  ├─ CRRobotRos2: ~124 个 ROS2 服务 → parseTool → TCP 指令字符串 → 机器人
  ├─ 发布: /joint_states_robot, /dobot_msgs_v4/msg/RobotStatus,
  │        /dobot_msgs_v4/msg/ToolVectorActual
  └─ ~124 个服务: /dobot_bringup_ros2/srv/<Name>

dobot_moveit (Python)
  ├─ joint_states.py: 转发 /joint_states_robot → /joint_states（MoveIt 标准）
  └─ action_move_server.py: FollowJointTrajectory action → ServoJ 服务调用
```

### ROS2 接口一览

| 话题 / 服务 | 类型 | 发布者 / 服务端 | 说明 |
|------------|------|----------------|------|
| `/joint_states_robot` | `sensor_msgs/JointState` | `dobot_bringup_v4` | 真机关节反馈 |
| `/joint_states` | `sensor_msgs/JointState` | `dobot_moveit`（转发） | MoveIt 标准关节状态 |
| `/dobot_msgs_v4/msg/RobotStatus` | `dobot_msgs_v4/RobotStatus` | `dobot_bringup_v4` | 机器人使能/连接状态 |
| `/dobot_msgs_v4/msg/ToolVectorActual` | `dobot_msgs_v4/ToolVectorActual` | `dobot_bringup_v4` | 末端位姿 (x,y,z,rx,ry,rz) |
| `/{robot}_group_controller/follow_joint_trajectory` | `control_msgs/FollowJointTrajectory` | `dobot_moveit` | MoveIt 轨迹执行 |
| `/dobot_bringup_ros2/srv/<Name>` | `dobot_msgs_v4/srv/<Name>` | `dobot_bringup_v4` | ~124 个机器人控制服务 |

---

## 项目结构

```
DOBOT_6Axis_ROS2_V4/
├── dobot_bringup_v4/        # C++ TCP/IP 驱动，~124 个服务 + 反馈发布
├── dobot_moveit/            # Python: MoveIt action server + 关节状态转发
├── dobot_rviz/              # RViz 可视化 + 共享 URDF/meshes（所有 *_moveit 的依赖）
├── dobot_gazebo/            # Gazebo 仿真 launch + 自定义 controller_loader
├── dobot_demo/              # 运动演示脚本（MovJ/MovL/DO）
├── dobot_msgs_v4/           # ROS2 msg/srv 定义（2 msg + ~124 srv）
├── servo_action/            # 关节轨迹 action client 演示
├── cr3_moveit/              # CR3 MoveIt 配置（URDF/SRDF/YAML/launch）
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
├── cra_description/         # Gazebo 仿真用 Xacro + meshes
├── misc/                    # FK.py、TF.py 辅助脚本 + TCP/IP 协议 PDF
├── docs/                    # 安装指南（INSTALL_JAZZY.md）
├── V4新增指令/              # V4 新增指令文档（中文）
├── image/                   # 文档图片
├── README.md
└── README_ZH.md
```

### 包依赖关系

```
dobot_rviz ──(URDF/meshes)──▶ *_moveit (12 个包) ──▶ dobot_moveit
cra_description ──(xacro)──▶ dobot_gazebo
dobot_msgs_v4 ──(msg/srv)──▶ dobot_bringup_v4, dobot_moveit, dobot_demo, servo_action
```

> **注意：** `dobot_rviz` 不仅用于可视化。其 `urdf/*.urdf` 文件被每个 `*_moveit` 配置通过 `$(find dobot_rviz)/urdf/<model>_robot.urdf` 引用。删除它会导致所有 MoveIt 包编译失败。

---

## 驱动配置

真机驱动读取 `dobot_bringup_v4/config/param.json`：

```json
{
  "robot_number": 1,
  "current_robot": 1,
  "node_info": [
    {
      "ip_address": "192.168.5.1",
      "robot_type": "cr5",
      "trajectory_duration": 0.3,
      "robot_node_name": "dobot_bringup_ros2"
    }
  ]
}
```

环境变量优先级高于 `param.json`：
- `DOBOT_TYPE` 覆盖 `robot_type`
- `IP_address` 覆盖 `ip_address`

多机器人场景下，增大 `robot_number` 并在 `node_info` 中添加对应条目，`current_robot` 指定当前启动的机器人。

---

## 支持型号

| 系列      | 型号                                         |
| --------- | -------------------------------------------- |
| CR 系列   | CR3、CR5、CR7、CR10、CR12、CR16、CR20、CR30H |
| CRAF 系列 | CR10AF                                       |
| E 系列    | E6 / ME6                                     |
| Nova 系列 | Nova2、Nova5                                 |

---

## 启动文件

| 启动文件 | 包名 | 说明 |
| -------- | ---- | ---- |
| `dobot_bringup_ros2.launch.py` | `dobot_bringup_v4` | 启动机器人驱动（硬件） |
| `dobot_rviz.launch.py` | `dobot_rviz` | RViz 可视化（仅查看模型） |
| `moveit_demo.launch.py` | `dobot_moveit` | MoveIt 虚拟演示（虚拟控制器） |
| `dobot_moveit.launch.py` | `dobot_moveit` | MoveIt 控制界面（真机，含 action_server） |
| `moveit_gazebo.launch.py` | `dobot_moveit` | MoveIt 端 Gazebo 联动 |
| `dobot_gazebo.launch.py` | `dobot_gazebo` | Gazebo 仿真（机器人 + ros2_control） |
| `sim.launch.py` | `dobot_gazebo` | Gazebo + MoveIt 一键联动 |

---

## 注意事项

> ⚠️ **安全第一**：运行前确保机器人在安全位置

1. 确保电脑 IP 与机器人在同一网段（192.168.X.X）
2. 确保端口 29999 和 30004 未被占用
3. 机器人需处于远程 TCP/IP 控制模式

---

## 故障排查

| 现象 | 原因 / 解决方案 |
|------|----------------|
| `EnableRobot service not available` | 驱动未启动。先执行场景 5 第 2 步，或检查 `IP_address` 是否可达（`ping 192.168.5.1`）。 |
| RViz 模型上下跳动 | 局域网内另一台机器发布了冲突的 `/tf`。`dobot_rviz.launch.py` 已设置 `ROS_LOCALHOST_ONLY=1`，请勿覆盖。 |
| MoveIt 报 `GOAL_TOLERANCE_VIOLATED` | 轨迹执行超时。调大 `trajectory_execution_timeout`（默认 120s），见 `dobot_moveit/launch/dobot_joint.launch.py`。 |
| Gazebo 控制器加载失败 | `controller_loader.py` 轮询硬件接口超时，多为 GPU/CPU 性能不足。用 `gui:=false` 启动降低负载。 |
| 编译 `dobot_msgs_v4` 失败 | 先单独编译它：`colcon build --packages-select dobot_msgs_v4`，再编译其余包。 |
| 服务调用返回 `res != 0` | 机器人报错。调用 `ros2 service call /dobot_bringup_ros2/srv/ClearError dobot_msgs_v4/srv/ClearError` 清错后重试。 |

---

## 版本信息

| 信息     | 内容                  |
| -------- | --------------------- |
| 当前版本 | V4.6.5                |
| ROS 版本 | ROS2 Jazzy            |
| 协议版本 | Dobot TCP/IP V4.6.5   |
| 仿真引擎 | Gazebo Sim (Harmonic) |

---

## 许可证

[MIT License](LICENSE)

<div align="center">
Built by Dobot-Arm
</div>
