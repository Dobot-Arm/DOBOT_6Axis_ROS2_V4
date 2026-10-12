

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/clock.hpp>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sensor_msgs/msg/joint_state.hpp>
#include <dobot_msgs_v4/msg/robot_status.hpp>
#include <dobot_msgs_v4/msg/tool_vector_actual.hpp>
#include <dobot_bringup/cr_robot_ros2.h>

int main(int argc, char *argv[])
{
  rclcpp::init(argc, argv);
  auto robot = std::make_shared<CRRobotRos2>();

  // 创建关节状态消息和发布者
  sensor_msgs::msg::JointState joint_state_msg;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_state_pub = robot->create_publisher<sensor_msgs::msg::JointState>("joint_states_robot", 10);
  joint_state_msg.name = {"joint1", "joint2", "joint3", "joint4", "joint5", "joint6"};
  joint_state_msg.position = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  dobot_msgs_v4::msg::RobotStatus robot_status_msg;
  rclcpp::Publisher<dobot_msgs_v4::msg::RobotStatus>::SharedPtr robot_status_pub = robot->create_publisher<dobot_msgs_v4::msg::RobotStatus>("dobot_msgs_v4/msg/RobotStatus", 10);

  dobot_msgs_v4::msg::ToolVectorActual tool_vector_actual_msg;
  rclcpp::Publisher<dobot_msgs_v4::msg::ToolVectorActual>::SharedPtr tool_vector_pub = robot->create_publisher<dobot_msgs_v4::msg::ToolVectorActual>("dobot_msgs_v4/msg/ToolVectorActual", 10);

  std::string z = "/";
  const char *robot_type = getenv("DOBOT_TYPE");
  std::string a = robot_type == nullptr ? "cr5" : robot_type;
  std::string b = "_robot/joint_controller/follow_joint_trajectory";
  std::string ss = z + a + b;
//  for (uint32_t i = 0; i < 6; i++)
//  {
//    joint_state_msg.position.push_back(0.0);
//    joint_state_msg.name.push_back(std::string("joint") + std::to_string(i + 1));
//  }

  double rate_value = robot->declare_parameter<double>("JointStatePublishRate", 10.0);

  robot->init();

  rclcpp::Rate rate(rate_value);
  double position[6];
  // A gap in the tool pose longer than 2.5 publish periods is logged: the arm camera's TF
  // comes from it, so frames captured in the gap cannot be placed. The log
  // says whether the loop was held up (its callbacks: the driver's services)
  // or had no fresh realtime data to publish.
  using Clock = std::chrono::steady_clock;
  const auto gap_warn = std::chrono::milliseconds(static_cast<int>(2500.0 / rate_value));
  Clock::time_point last_tool_pub{};
  Clock::duration longest_spin{};
  int stale_passes = 0;
  auto ms = [](Clock::duration d) {
    return static_cast<long>(std::chrono::duration_cast<std::chrono::milliseconds>(d).count());
  };
  while (rclcpp::ok())
  {
    // Joint states only from fresh realtime data: without it nothing is
    // published, so TF and MoveIt keep no pose rather than a made-up one.
    if (robot->getJointState(position)) {
      joint_state_msg.header.stamp = robot->get_clock()->now();
      joint_state_msg.header.frame_id = "dummy_link";
      for (uint32_t i = 0; i < 6; i++)
      {
        joint_state_msg.position[i] = position[i];
      }
      joint_state_pub->publish(joint_state_msg);
    }

    double val[6];
    int user = 0, tool = 0;
    if (robot->getToolVectorActual(val, &user, &tool)) {
      tool_vector_actual_msg.header.stamp = robot->get_clock()->now();
      tool_vector_actual_msg.header.frame_id = "base_link";
      tool_vector_actual_msg.user = static_cast<int8_t>(user);
      tool_vector_actual_msg.tool = static_cast<int8_t>(tool);
      tool_vector_actual_msg.x = val[0];
      tool_vector_actual_msg.y = val[1];
      tool_vector_actual_msg.z = val[2];
      tool_vector_actual_msg.rx = val[3];
      tool_vector_actual_msg.ry = val[4];
      tool_vector_actual_msg.rz = val[5];
      tool_vector_pub->publish(tool_vector_actual_msg);
      auto now = Clock::now();
      if (last_tool_pub != Clock::time_point{} && now - last_tool_pub > gap_warn)
        RCLCPP_WARN(robot->get_logger(),
                    "Tool pose not published for %ld ms: %d loop pass(es) without fresh "
                    "realtime data, longest callback pass %ld ms",
                    ms(now - last_tool_pub), stale_passes, ms(longest_spin));
      last_tool_pub = now;
      longest_spin = Clock::duration{};
      stale_passes = 0;
    } else {
      ++stale_passes;
    }

    // publish robot status
    robot_status_msg.is_enable = robot->isEnable();
    robot_status_msg.is_connected = robot->isConnected();
    robot_status_pub->publish(robot_status_msg);
    auto spin_start = Clock::now();
    rclcpp::spin_some(robot);
    longest_spin = std::max(longest_spin, Clock::now() - spin_start);
    rate.sleep();
  }

  // 关闭 ROS 2 环境
  rclcpp::shutdown();
  return 0;
}
