# <center>ROS-Robot</center>
# 1. 简介

DOBOT_6Axis-ROS2_V4 是为 Dobot 公司旗下基于TCP/IP协议的ROS的封装设计的软件开发套件。它基于 ROS/C++、python语言开发，遵循Dobot-TCP-IP控制通信协议，通过socket与机器终端进行Tcp连接，  并为用户提供了易用的api接口。通过 DOBOT_6Axis-ROS2_V4，用户可以快速地连接Dobot机器并进行二次开发对机器的控制与使用。



## 前置依赖

* 电脑可用网线连接控制器的网口，然后设置固定 IP，与控制器 IP 在同一网段下。也可无线连接控制器。

  有线连接时连接：ip为192.168.5.1,  无线连接：ip为192.168.1.6

* 尝试 ping 通控制器 IP，确保在同一网段下。
* Ubuntu 22.04

### 运行步骤

# 1. 源码编译

### 下载源码

```
mkdir -p ~/dobot_ws/src

cd ~/dobot_ws/src

git clone https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git

cd ~/catkin_ws
```

### 编译

```
colcon build

source install/local_setup.sh
```
### 设置环境变量

```
echo "source ~/dobot_ws/install/local_setup.sh" >> ~/.bashrc
```

### 设置机械臂连接IP
```
echo "export IP_address=192.168.5.1" >> ~/.bashrc
source ~/.bashrc
```
### 若为 CR3 机械臂，则使用如下命令设置机械臂类型

```
echo "export DOBOT_TYPE=cr3" >> ~/.bashrc
source ~/.bashrc
```

### 若为 CR5 机械臂，则使用如下命令设置机械臂类型

```
echo "export DOBOT_TYPE=cr5" >> ~/.bashrc
source ~/.bashrc
```

### 若为 CR10 机械臂，则使用如下命令设置机械臂类型

```
echo "export DOBOT_TYPE=cr10" >> ~/.bashrc
source ~/.bashrc
```

### 若为 CR16 机械臂，则使用如下命令设置机械臂类型

```
echo "export DOBOT_TYPE=cr16" >> ~/.bashrc
source ~/.bashrc
```
### 若为 E6 机械臂，则使用如下命令设置机械臂类型

```
echo "export DOBOT_TYPE=me6" >> ~/.bashrc
source ~/.bashrc
```
# 2. 使用演示

## 在仿真环境下使用

## rviz 显示

```
ros2 launch dobot_rviz dobot_rviz.launch.py
```

可通过加载 joint_state_publisher_gui 调节各关节的角度，在 rviz 上看到其显示效果

![rviz显示](/image/rviz.jpg)

## moveit 控制

* 使用如下命令启动 moveit

```
ros2 launch dobot_moveit dobot_moveit.launch.py
```

* 鼠标将关节拖到任意的角度，点击 "Plan and Execute" 即可看到运行效果

![moveit显示](/image/moveit.jpg)

## gazebo 仿真

* 使用如下命令启动 gazebo

```
ros2 launch dobot_gazebo dobot_gazebo.launch.py 
```
![gazebo仿真](/image/gazebo.jpg)

##  控制真实机械臂

* **使用如下命令连接机械臂**

  ```
  ros2 launch cr_robot_ros2 dobot_bringup_ros2.launch.py
  ```

* **使用如下命令查看服务**
```
ros2 service list
```
![service](/image/service.jpg)
