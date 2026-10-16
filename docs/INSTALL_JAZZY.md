# DOBOT 6Axis ROS2 V4 — Ubuntu 24.04 + ROS2 Jazzy 完整安装指南

本文档提供从零开始在 Ubuntu 24.04 LTS 上安装 ROS2 Jazzy 并运行 DOBOT 6Axis 仿真环境的完整步骤。

---

## 目录

- [1. 安装 Ubuntu 24.04 LTS](#1-安装-ubuntu-2404-lts)
- [2. 系统基础配置](#2-系统基础配置)
- [3. 安装 ROS2 Jazzy](#3-安装-ros2-jazzy)
- [4. 安装 Gazebo Sim (Harmonic)](#4-安装-gazebo-sim-harmonic)
- [5. 安装 ros2_control](#5-安装-ros2_control)
- [6. 安装项目依赖](#6-安装项目依赖)
- [7. 创建工作空间并编译](#7-创建工作空间并编译)
- [8. 网络配置（连接真实机械臂）](#8-网络配置连接真实机械臂)
- [9. 运行验证](#9-运行验证)
- [10. 常用问题排查](#10-常用问题排查)

---

## 1. 安装 Ubuntu 24.04 LTS

### 1.1 下载系统镜像

从 Ubuntu 官网下载 24.04 LTS (Noble Numbat) 桌面版：

```
https://releases.ubuntu.com/noble/
```

推荐使用 `ubuntu-24.04.x-desktop-amd64.iso`

### 1.2 制作启动盘并安装

使用 Rufus (Windows) 或 balenaEtcher 制作 USB 启动盘，然后按常规流程安装系统。

**硬件建议：**

- CPU: 4核及以上
- 内存: 8GB 及以上（推荐 16GB）
- 磁盘: 50GB 以上可用空间
- 显卡: 支持 OpenGL 3.3+（Gazebo Sim 需要）

---

## 2. 系统基础配置

### 2.1 更新系统

```bash
sudo apt update
sudo apt upgrade -y
sudo snap refresh
```

### 2.2 设置 locale（UTF-8 支持）

```bash
sudo apt install locales -y
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8
```

验证：

```bash
locale  # 确保输出中包含 UTF-8
```

### 2.3 启用 Universe 仓库

```bash
sudo apt install software-properties-common -y
sudo add-apt-repository universe -y
```

### 2.4 安装开发工具

```bash
sudo apt install -y \
  build-essential \
  cmake \
  git \
  curl \
  wget \
  python3-pip \
  python3-colcon-common-extensions
```

---

## 3. 安装 ROS2 Jazzy

### 3.1 添加 ROS2 apt 仓库

```bash
# 添加 ROS2 GPG 密钥和仓库
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
  http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | \
  sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
```

### 3.2 安装 ROS2 Jazzy 桌面版

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install ros-jazzy-desktop -y
```

### 3.3 安装 ros-dev-tools

```bash
sudo apt install ros-dev-tools -y
```

### 3.4 配置环境变量

```bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

### 3.5 验证 ROS2 安装

```bash
# 打开终端1，启动 talker
ros2 run demo_nodes_cpp talker

# 打开终端2，启动 listener
ros2 run demo_nodes_py listener
```

如果能正常收发消息，则 ROS2 安装成功。

---

## 4. 安装 Gazebo Sim (Harmonic)

ROS2 Jazzy 默认使用 Gazebo Sim (Harmonic)，不再兼容旧版 Gazebo Classic。

### 4.1 安装

```bash
sudo apt install ros-jazzy-ros-gz -y
```

### 4.2 验证 Gazebo 安装

```bash
gz sim -v 4 shapes.sdf
```

如果能正常显示 Gazebo Sim 窗口并加载模型，则安装成功。

> **注意：** 如果出现崩溃或黑屏，参考 [10. 常用问题排查](#10-常用问题排查)。

---

## 5. 安装 ros2_control

```bash
sudo apt install -y \
  ros-jazzy-ros2-control \
  ros-jazzy-ros2-controllers \
  ros-jazzy-gz-ros2-control \
  ros-jazzy-joint-state-publisher \
  ros-jazzy-robot-state-publisher \
  ros-jazzy-ros-gz-bridge
```

---

## 6. 安装项目依赖

### 6.1 MoveIt 及相关包

```bash
sudo apt install -y \
  ros-jazzy-moveit \
  ros-jazzy-warehouse-ros-sqlite \
  ros-jazzy-joint-trajectory-controller \
  ros-jazzy-joint-state-broadcaster
```

### 6.2 可视化工具

```bash
sudo apt install -y \
  ros-jazzy-rviz2 \
  ros-jazzy-rviz-common \
  ros-jazzy-rviz-default-plugins
```

### 6.3 其他依赖

```bash
sudo apt install -y \
  ros-jazzy-xacro \
  ros-jazzy-tf2-ros \
  ros-jazzy-controller-manager \
  ros-jazzy-ros2launch
```

### 6.4 Python 依赖

```bash
pip3 install --user setuptools
```

---

## 7. 创建工作空间并编译

### 7.1 创建 ROS2 工作空间

```bash
mkdir -p ~/dobot_ws/src
cd ~/dobot_ws/src
```

### 7.2 克隆项目代码

```bash
git clone -b jazzy https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
```

### 7.3 编译

```bash
cd ~/dobot_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
```

如果遇到特定包编译失败，可以单独跳过：

```bash
colcon build --symlink-install --packages-skip <问题包名>
```

### 7.4 配置工作空间环境

```bash
echo "source ~/dobot_ws/install/local_setup.bash" >> ~/.bashrc
source ~/.bashrc
```

---

## 8. 网络配置（连接真实机械臂）

如果只需仿真测试，可跳过此节。

### 8.1 设置本机 IP

```bash
# 将本机有线网卡 IP 设置为与机器人同一网段，例如：
sudo ip addr add 192.168.5.100/24 dev eth0
```

或通过 Ubuntu 网络设置 GUI 配置静态 IP：

- IP: `192.168.5.x`（x 为 2~254，且不与机器人冲突）
- 子网掩码: `255.255.255.0`

### 8.2 验证连通性

```bash
ping 192.168.5.1
```

### 8.3 检查端口占用

```bash
# 确保 29999 和 30004 端口未被占用
sudo lsof -i :29999
sudo lsof -i :30004
```

### 8.4 设置 DOBOT_TYPE 环境变量

```bash
# 根据实际机器人型号设置，例如：
export DOBOT_TYPE=cr10
# 可选型号：cr3 cr5 cr7 cr10 cr10af cr12 cr16 cr20 cr30h me6 nova2 nova5

# 写入 bashrc 持久化
echo "export DOBOT_TYPE=cr10" >> ~/.bashrc
```

### 8.5 设置机器人 IP

编辑 `~/dobot_ws/src/DOBOT_6Axis_ROS2_V4/dobot_bringup_v4/config/param.json`，确保 IP 地址正确。

或者通过环境变量：

```bash
export IP_address=192.168.5.1
echo "export IP_address=192.168.5.1" >> ~/.bashrc
```

---

## 9. 运行验证

### 9.1 前置条件

每次打开新终端，确保已设置 DOBOT_TYPE：

```bash
export DOBOT_TYPE=cr10  # 替换为你的型号
```

### 9.2 RViz 可视化（不需要 Gazebo）

```bash
ros2 launch dobot_rviz dobot_rviz.launch.py
```

### 9.3 MoveIt 虚拟演示（不需要 Gazebo）

```bash
ros2 launch dobot_moveit moveit_demo.launch.py
```

### 9.4 Gazebo Sim 仿真

```bash
ros2 launch dobot_gazebo dobot_gazebo.launch.py
```

### 9.5 控制真实机械臂

```bash
# 终端1：启动驱动
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# 终端2：启动 MoveIt
ros2 launch dobot_moveit dobot_moveit.launch.py
```

---

## 10. 常用问题排查

### 10.1 Gazebo Sim 启动黑屏或崩溃

```bash
# 设置 XCB 平台（解决某些 NVIDIA 显卡问题）
export QT_QPA_PLATFORM=xcb

# 或在 ~/.bashrc 中永久设置
echo 'export QT_QPA_PLATFORM=xcb' >> ~/.bashrc

# 重试
gz sim -v 4 shapes.sdf
```

### 10.2 colcon build 编译失败

**常见原因 1：缺少 ROS 环境**

```bash
source /opt/ros/jazzy/setup.bash
```

**常见原因 2：缺少依赖**

```bash
cd ~/dobot_ws
rosdep install --from-paths src --ignore-src -r -y
```

**常见原因 3：消息包未编译**

```bash
# 先单独编译消息包
colcon build --packages-select dobot_msgs_v4 --symlink-install
source install/local_setup.bash
# 再编译整个工作空间
colcon build --symlink-install
```

### 10.3 找不到 launch 文件或包

```bash
# 确保 setup 脚本已 source
source ~/dobot_ws/install/local_setup.bash

# 检查包是否可见
ros2 pkg list | grep dobot
```

### 10.4 仿真中机器人不响应

确保在仿真启动后加载控制器：

```bash
# 检查控制器状态
ros2 control list_controllers

# 如果 controller 未加载，手动加载
ros2 control load_controller --set-state active joint_state_broadcaster
ros2 control load_controller --set-state active ${DOBOT_TYPE}_group_controller
```

### 10.5 真实机械臂连接不上

1. 确认机器人和电脑在同一网段（`ping 192.168.5.1`）
2. 确认机器人处于 **远程 TCP/IP 控制模式**
3. 确认端口 29999、30004 未被占用
4. 检查防火墙设置：`sudo ufw disable`（测试用）

### 10.6 warehouse_ros 数据库错误

Jazzy 中 MongoDB 已废弃，本仓库已切换到 `warehouse_ros_sqlite`。如果遇到数据库相关错误：

```bash
# 确保安装了 SQLite 版本
sudo apt install ros-jazzy-warehouse-ros-sqlite

# 清理旧的 MongoDB 数据（如果存在）
rm -rf ~/.ros/warehouse_db
```

---

## 参考链接

- [ROS2 Jazzy 官方安装文档](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html)
- [Gazebo Sim (Harmonic) 文档](https://gazebosim.org/docs/harmonic)
- [MoveIt 2 文档](https://moveit.ai/)
- [ros2_control 文档](https://control.ros.org/master/index.html)
- [DOBOT 6Axis ROS2 V4 GitHub](https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4)
