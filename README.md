**ROS2_V4 安装与配置指南**

---

### **简介**  
DOBOT_6Axis-ROS2_V4 是 Dobot 公司基于 TCP/IP 协议开发的 ROS 软件开发套件（SDK）。该套件采用 ROS/C++ 和 Python 语言开发，遵循 Dobot-TCP-IP 控制通信协议，通过 Socket 与机械臂终端建立 TCP 连接，并提供简洁的 API 接口。用户可通过此套件快速连接 Dobot 机械臂，实现二次开发及控制功能。

---

### **前置条件**  
1. **网络配置**  
   - **有线连接**：控制器 IP 为 `192.168.5.1`，需将电脑设置为同一网段的固定 IP。  
   - **无线连接**：控制器 IP 为 `192.168.1.6`。  
   - 使用 `ping` 命令测试控制器 IP 是否可达，确保网络连通性。  

2. **系统要求**  
   - **操作系统**：Ubuntu 22.04  
   - **ROS 版本**：ROS2 Humble  

---

### **安装与配置步骤**  

#### **1. 源码编译**  
1. 下载源码：  
   ```bash
   mkdir -p ~/dobot_ws/src
   cd ~/dobot_ws/src
   git clone https://github.com/Dobot-Arm/DOBOT_6Axis_ROS2_V4.git
   cd ~/dobot_ws
   ```

2. 编译源码：  
   ```bash
   colcon build
   source install/local_setup.sh
   ```

3. 设置环境变量：  
   ```bash
   echo "source ~/dobot_ws/install/local_setup.sh" >> ~/.bashrc
   ```

4. 配置机械臂连接 IP（默认为有线连接 IP）：  
   ```bash
   echo "export IP_address=192.168.5.1" >> ~/.bashrc
   ```

5. 指定机械臂型号（根据实际型号选择）：  
   ```bash
   # 示例：CR5 型号
   echo "export DOBOT_TYPE=cr5" >> ~/.bashrc
   ```
   - 支持型号：CR3、CR5、CR7、CR10、CR12、CR16、CR20、E6（ME6）。  

6. 生效配置：  
   ```bash
   source ~/.bashrc
   ```
   - 如需修改配置，可通过文本编辑器编辑 `~/.bashrc` 文件。  

---

### **功能演示**  

#### **1. 仿真环境使用**  
- **RViz 模型加载**：  
  ```bash
  ros2 launch dobot_rviz dobot_rviz.launch.py
  ```
  - 用于可视化机械臂模型。  
![rviz](/image/rviz.jpg)
- **MoveIt 虚拟演示**：  
  ```bash
  ros2 launch dobot_moveit moveit_demo.launch.py
  ```
  - 拖动关节角度后，点击 "Plan and Execute" 查看运行效果。  
![moveit](/image/moveit.jpg)
- **Gazebo 仿真**：  
  ```bash
  ros2 launch dobot_gazebo dobot_gazebo.launch.py
  ```
  - 启动 Gazebo 仿真环境。  
![gazebo](/image/gazebo.jpg)
- **Gazebo 与 MoveIt 联动**：  
  1. 启动 Gazebo 和 MoveIt：  
     ```bash
     ros2 launch dobot_gazebo gazebo_moveit.launch.py
     ros2 launch dobot_moveit moveit_gazebo.launch.py
     ```
  2. 在 MoveIt 中拖动机械臂并执行 "Plan and Execute"，动作将同步至 Gazebo。  
  ![service](/image/node.jpg)
---

#### **2. 控制真实机械臂**  
1. **连接机械臂**：  
   ```bash
   ros2 launch cr_robot_ros2 dobot_bringup_ros2.launch.py
   ```

2. **查看服务列表**：  
   ```bash
   ros2 service list
   ```
   - ![service](/image/service.jpg)

3. **使用 MoveIt 控制机械臂**：  
   ```bash
   ros2 launch dobot_moveit dobot_moveit.launch.py
   ```
   ![service](/image/dobot_moveit.jpg)
   - **注意**：需确保机械臂处于远程 TCP 模式并已上使能。  

4. **使能机械臂**：  
   - 使用 `rqt service caller` 调用服务：  
     ```
     /dobot_bringup_ros2/srv/EnableRobot
     ```
     ![gazebo](/image/rqt.jpg)

5. **关键话题**：  
   - `/joint_states_robot`：关节角度信息。  
   - `/dobot_msgs_v4/msg/ToolVectorActual`：笛卡尔位置信息。  
   ![gazebo](/image/topic.jpg)
---

### **必要工具包安装**  

#### **1. Gazebo 安装**  
- 安装命令：  
  ```bash
  sudo apt install ros-humble-gazebo-*
  ```
- 环境变量配置：  
  ```bash
  echo "source /usr/share/gazebo/setup.bash" >> ~/.bashrc
  ```
- 测试运行：  
  ```bash
  ros2 launch gazebo_ros gazebo.launch.py
  ```

#### **2. MoveIt 安装**  
- 安装命令：  
  ```bash
  sudo apt-get install ros-humble-moveit
  ```

---

### **注意事项**  
- 确保网络配置正确，避免因 IP 设置问题导致连接失败。  
- 操作真实机械臂时，需严格遵守安全规范，避免意外动作。  
- 仿真环境下建议先测试动作规划，再应用于真实场景。  

--- 

**最后更新日期**：2025年4月2日