<div align="center">

 <img src="image/dobot_moveit.jpg" alt="DOBOT 6Axis ROS2 V4" style="max-width: 600px; margin-bottom: 20px;" />

 <h1>DOBOT 6Axis ROS2 V4</h1>

 **Dobot Robotics ROS2 Software Development Kit**  
 High-performance robot control framework based on TCP/IP protocol

 [English](README.md) · [简体中文](README_ZH.md)

 [![Platform](https://img.shields.io/badge/Platform-Ubuntu%2022.04-blue?style=flat-square)](https://ubuntu.com/download/server)
 [![ROS](https://img.shields.io/badge/ROS2-Humble-green?style=flat-square)](https://docs.ros.org/en/humble/)
 [![License](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

</div>

---

## Quick Start

### System Requirements

| Requirement | Version |
|-------------|---------|
| OS | Ubuntu 22.04 LTS |
| ROS Version | ROS2 Humble |
| Python | 3.8+ |

### Network Configuration

| Configuration | Description |
|---------------|-------------|
| Robot IP | 192.168.5.1 (must be in the same subnet) |
| Control Port | 29999 |
| Feedback Port | 30004 |

### Installation

```bash
# Create workspace
mkdir -p ~/dobot_ws/src
cd ~/dobot_ws/src
git clone -b feature/v4-optimization https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
cd ~/dobot_ws

# Install dependencies
sudo apt update && sudo apt install -y \
  ros-humble-moveit \
  ros-humble-gazebo-* \
  ros-humble-joint-state-publisher \
  ros-humble-robot-state-publisher

# Build
colcon build --symlink-install
source install/local_setup.sh
```

---

## Feature Demonstrations

```bash
# RViz Visualization
ros2 launch dobot_rviz dobot_rviz.launch.py

# MoveIt Virtual Demo
ros2 launch dobot_moveit moveit_demo.launch.py

# Gazebo Simulation
ros2 launch dobot_gazebo dobot_gazebo.launch.py

# Control Real Robot
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py
ros2 launch dobot_moveit dobot_moveit.launch.py
```

---

## Project Structure

```
DOBOT_6Axis_ROS2_V4/
├── dobot_bringup_v4/        # Robot Driver
├── dobot_moveit/            # MoveIt Core
├── dobot_rviz/              # RViz Visualization
├── dobot_gazebo/            # Gazebo Simulation
├── cr3_moveit/              # CR3 Configuration
├── cr5_moveit/              # CR5 Configuration
├── cr7_moveit/              # CR7 Configuration
├── cr10_moveit/             # CR10 Configuration
├── cr10af_moveit/           # CR10AF Configuration
├── cr12_moveit/             # CR12 Configuration
├── cr16_moveit/             # CR16 Configuration
├── cr20_moveit/             # CR20 Configuration
├── cr30h_moveit/            # CR30H Configuration
├── me6_moveit/              # E6/ME6 Configuration
├── nova2_moveit/            # Nova2 Configuration
├── nova5_moveit/            # Nova5 Configuration
├── cra_description/         # Robot Description
├── misc/                    # Auxiliary Tools & Docs
├── image/                   # Documentation Images
├── README.md
└── README_ZH.md
```

---

## Supported Models

| Series | Models |
|--------|--------|
| CR Series | CR3, CR5, CR7, CR10, CR12, CR16, CR20, CR30H |
| CRAF Series | CR10AF |
| E Series | E6 / ME6 |
| Nova Series | Nova2, Nova5 |

---

## Launch Files

| Launch File | Description |
|-------------|-------------|
| `dobot_bringup_ros2.launch.py` | Launch robot driver |
| `dobot_rviz.launch.py` | Launch RViz |
| `moveit_demo.launch.py` | MoveIt virtual demo |
| `dobot_moveit.launch.py` | MoveIt control interface |
| `dobot_gazebo.launch.py` | Launch Gazebo |
| `gazebo_moveit.launch.py` | Gazebo-MoveIt integration |

---

## Notes

> ⚠️ **Safety First**: Ensure the robot is in a safe position before operation

1. Ensure computer IP is in the same subnet as robot (192.168.X.X)
2. Ensure ports 29999 and 30004 are not occupied
3. Robot must be in remote TCP/IP control mode

---

## Version Information

| Information | Content |
|-------------|---------|
| Current Version | V4.6.5 |
| ROS Version | ROS2 Humble |
| Protocol Version | Dobot TCP/IP V4.6.5 |

---

## License

[MIT License](LICENSE)

<div align="center">
Built by Dobot-Arm
</div>
