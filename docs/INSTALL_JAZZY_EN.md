# DOBOT 6Axis ROS2 V4 — Complete Installation Guide for Ubuntu 24.04 + ROS2 Jazzy

This document provides step-by-step instructions for installing ROS2 Jazzy on Ubuntu 24.04 LTS and running the DOBOT 6Axis simulation environment from scratch.

---

## Table of Contents

- [1. Install Ubuntu 24.04 LTS](#1-install-ubuntu-2404-lts)
- [2. Basic System Configuration](#2-basic-system-configuration)
- [3. Install ROS2 Jazzy](#3-install-ros2-jazzy)
- [4. Install Gazebo Sim (Harmonic)](#4-install-gazebo-sim-harmonic)
- [5. Install ros2_control](#5-install-ros2_control)
- [6. Install Project Dependencies](#6-install-project-dependencies)
- [7. Create Workspace and Build](#7-create-workspace-and-build)
- [8. Network Configuration (Real Robot)](#8-network-configuration-real-robot)
- [9. Running and Verification](#9-running-and-verification)
- [10. Troubleshooting](#10-troubleshooting)

---

## 1. Install Ubuntu 24.04 LTS

### 1.1 Download the System Image

Download the 24.04 LTS (Noble Numbat) Desktop edition from the Ubuntu official website:

```
https://releases.ubuntu.com/noble/
```

Recommended: `ubuntu-24.04.x-desktop-amd64.iso`

### 1.2 Create Bootable USB and Install

Use Rufus (Windows) or balenaEtcher to create a bootable USB drive, then follow the standard installation process.

**Hardware Recommendations:**

- CPU: 4 cores or more
- RAM: 8GB or more (16GB recommended)
- Disk: 50GB+ free space
- GPU: OpenGL 3.3+ support (required by Gazebo Sim)

---

## 2. Basic System Configuration

### 2.1 Update System

```bash
sudo apt update
sudo apt upgrade -y
sudo snap refresh
```

### 2.2 Set Locale (UTF-8 Support)

```bash
sudo apt install locales -y
sudo locale-gen en_US en_US.UTF-8
sudo update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8
export LANG=en_US.UTF-8
```

Verify:

```bash
locale  # Ensure UTF-8 is present in the output
```

### 2.3 Enable Universe Repository

```bash
sudo apt install software-properties-common -y
sudo add-apt-repository universe -y
```

### 2.4 Install Development Tools

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

## 3. Install ROS2 Jazzy

### 3.1 Add ROS2 apt Repository

```bash
# Add ROS2 GPG key and repository
sudo apt update && sudo apt install curl -y
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
  -o /usr/share/keyrings/ros-archive-keyring.gpg

echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
  http://packages.ros.org/ros2/ubuntu $(. /etc/os-release && echo $UBUNTU_CODENAME) main" | \
  sudo tee /etc/apt/sources.list.d/ros2.list > /dev/null
```

### 3.2 Install ROS2 Jazzy Desktop

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install ros-jazzy-desktop -y
```

### 3.3 Install ros-dev-tools

```bash
sudo apt install ros-dev-tools -y
```

### 3.4 Configure Environment Variables

```bash
echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc
source ~/.bashrc
```

### 3.5 Verify ROS2 Installation

```bash
# Terminal 1: start talker
ros2 run demo_nodes_cpp talker

# Terminal 2: start listener
ros2 run demo_nodes_py listener
```

If messages are sent and received correctly, ROS2 is installed successfully.

---

## 4. Install Gazebo Sim (Harmonic)

ROS2 Jazzy uses Gazebo Sim (Harmonic) by default and is no longer compatible with the legacy Gazebo Classic.

### 4.1 Install

```bash
sudo apt install ros-jazzy-ros-gz -y
```

### 4.2 Verify Gazebo Installation

```bash
gz sim -v 4 shapes.sdf
```

If a Gazebo Sim window appears and models load correctly, the installation is successful.

> **Note:** If it crashes or shows a black screen, see [10. Troubleshooting](#10-troubleshooting).

---

## 5. Install ros2_control

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

## 6. Install Project Dependencies

### 6.1 MoveIt and Related Packages

```bash
sudo apt install -y \
  ros-jazzy-moveit \
  ros-jazzy-warehouse-ros-sqlite \
  ros-jazzy-joint-trajectory-controller \
  ros-jazzy-joint-state-broadcaster
```

### 6.2 Visualization Tools

```bash
sudo apt install -y \
  ros-jazzy-rviz2 \
  ros-jazzy-rviz-common \
  ros-jazzy-rviz-default-plugins
```

### 6.3 Other Dependencies

```bash
sudo apt install -y \
  ros-jazzy-xacro \
  ros-jazzy-tf2-ros \
  ros-jazzy-controller-manager \
  ros-jazzy-ros2launch
```

### 6.4 Python Dependencies

```bash
pip3 install --user setuptools
```

---

## 7. Create Workspace and Build

### 7.1 Create ROS2 Workspace

```bash
mkdir -p ~/dobot_ws/src
cd ~/dobot_ws/src
```

### 7.2 Clone Project Code

```bash
git clone -b jazzy https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
```

### 7.3 Build

```bash
cd ~/dobot_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
```

If a specific package fails to build, you can skip it:

```bash
colcon build --symlink-install --packages-skip <problem-package-name>
```

### 7.4 Configure Workspace Environment

```bash
echo "source ~/dobot_ws/install/local_setup.bash" >> ~/.bashrc
source ~/.bashrc
```

---

## 8. Network Configuration (Real Robot)

Skip this section if you only need simulation testing.

### 8.1 Set Local IP

```bash
# Set the wired network interface IP to the same subnet as the robot, e.g.:
sudo ip addr add 192.168.5.100/24 dev eth0
```

Or configure a static IP via the Ubuntu network settings GUI:

- IP: `192.168.5.x` (x is 2~254, must not conflict with the robot)
- Subnet mask: `255.255.255.0`

### 8.2 Verify Connectivity

```bash
ping 192.168.5.1
```

### 8.3 Check Port Availability

```bash
# Ensure ports 29999 and 30004 are not occupied
sudo lsof -i :29999
sudo lsof -i :30004
```

### 8.4 Set DOBOT_TYPE Environment Variable

```bash
# Set according to your actual robot model, e.g.:
export DOBOT_TYPE=cr10
# Available models: cr3 cr5 cr7 cr10 cr10af cr12 cr16 cr20 cr30h me6 nova2 nova5

# Persist to bashrc
echo "export DOBOT_TYPE=cr10" >> ~/.bashrc
```

### 8.5 Set Robot IP

Edit `~/dobot_ws/src/DOBOT_6Axis_ROS2_V4/dobot_bringup_v4/config/param.json` to ensure the IP address is correct.

Or via environment variable:

```bash
export IP_address=192.168.5.1
echo "export IP_address=192.168.5.1" >> ~/.bashrc
```

---

## 9. Running and Verification

### 9.1 Prerequisites

Each time you open a new terminal, ensure DOBOT_TYPE is set:

```bash
export DOBOT_TYPE=cr10  # Replace with your model
```

### 9.2 RViz Visualization (No Gazebo Required)

```bash
ros2 launch dobot_rviz dobot_rviz.launch.py
```

### 9.3 MoveIt Virtual Demo (No Gazebo Required)

```bash
ros2 launch dobot_moveit moveit_demo.launch.py
```

### 9.4 Gazebo Sim Simulation

```bash
ros2 launch dobot_gazebo dobot_gazebo.launch.py
```

### 9.5 Control Real Robot

```bash
# Terminal 1: Start driver
ros2 launch dobot_bringup_v4 dobot_bringup_ros2.launch.py

# Terminal 2: Start MoveIt
ros2 launch dobot_moveit dobot_moveit.launch.py
```

---

## 10. Troubleshooting

### 10.1 Gazebo Sim Black Screen or Crash on Startup

```bash
# Set XCB platform (fixes some NVIDIA GPU issues)
export QT_QPA_PLATFORM=xcb

# Or set permanently in ~/.bashrc
echo 'export QT_QPA_PLATFORM=xcb' >> ~/.bashrc

# Retry
gz sim -v 4 shapes.sdf
```

### 10.2 colcon build Fails

**Common Cause 1: Missing ROS environment**

```bash
source /opt/ros/jazzy/setup.bash
```

**Common Cause 2: Missing dependencies**

```bash
cd ~/dobot_ws
rosdep install --from-paths src --ignore-src -r -y
```

**Common Cause 3: Message packages not built**

```bash
# Build message package first
colcon build --packages-select dobot_msgs_v4 --symlink-install
source install/local_setup.bash
# Then build the entire workspace
colcon build --symlink-install
```

### 10.3 Cannot Find Launch Files or Packages

```bash
# Ensure setup script is sourced
source ~/dobot_ws/install/local_setup.bash

# Check if packages are visible
ros2 pkg list | grep dobot
```

### 10.4 Robot Not Responding in Simulation

Ensure controllers are loaded after simulation starts:

```bash
# Check controller status
ros2 control list_controllers

# If controller is not loaded, load manually
ros2 control load_controller --set-state active joint_state_broadcaster
ros2 control load_controller --set-state active ${DOBOT_TYPE}_group_controller
```

### 10.5 Cannot Connect to Real Robot

1. Verify the computer and robot are on the same subnet (`ping 192.168.5.1`)
2. Verify the robot is in **Remote TCP/IP Control Mode**
3. Verify ports 29999 and 30004 are not occupied
4. Check firewall settings: `sudo ufw disable` (for testing)

### 10.6 warehouse_ros Database Errors

MongoDB has been deprecated in Jazzy. This repository has switched to `warehouse_ros_sqlite`. If you encounter database errors:

```bash
# Ensure the SQLite version is installed
sudo apt install ros-jazzy-warehouse-ros-sqlite

# Clean up old MongoDB data (if exists)
rm -rf ~/.ros/warehouse_db
```

---

## References

- [ROS2 Jazzy Official Installation Guide](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html)
- [Gazebo Sim (Harmonic) Documentation](https://gazebosim.org/docs/harmonic)
- [MoveIt 2 Documentation](https://moveit.ai/)
- [ros2_control Documentation](https://control.ros.org/master/index.html)
- [DOBOT 6Axis ROS2 V4 GitHub](https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4)
