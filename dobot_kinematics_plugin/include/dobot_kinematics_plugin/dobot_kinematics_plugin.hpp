// Copyright (c) Dobot. BSD License.
// MoveIt2 运动学插件：FK/IK 委托给 Dobot 控制器 ROS 服务，KDL 兜底。
//
// 服务接口（dobot_bringup_v4 提供，已核对源码）：
//   PositiveKin: req{j1..j6,user,tool}     resp{robot_return="{x,y,z,rx,ry,rz}", res}
//   InverseKin:  req{x,y,z,rx,ry,rz,use_joint_near,joint_near,user,tool}
//                 resp{robot_return="{j1..j6}", res}
//   单位：控制器用 度(关节/姿态) + 毫米(位置)；res==0 表示成功。
//   依据：dobot_bringup_v4/src/command.cpp:53(deg2Rad)、command.cpp:164-213(字符串解析)。
//
// 分层策略：
//   - getPositionIK：脚本 setPoseTarget 调它 → solveIK（控制器优先 → KDL 兜底）
//   - searchPositionIK：OMPL 规划 + compute_ik 调它 → 直接 KDL（高频，要速度）
//   - getPositionFK：执行监测调它 → 始终 KDL（高频，要速度）
//   use_dobot_solver:=true 时建议同时禁用 MoveGroupKinematicsService 阻止拖拽。
//
// 注意：override 集合对齐 MoveIt2 Humble 的 kinematics::KinematicsBase。
//       若在其他 MoveIt2 版本上构建，searchPositionIK 带 IKCallbackFn 的重载签名可能
//       存在差异，编译器会以 override 报错指明，按报错调整即可。
#pragma once

#include <moveit/kinematics_base/kinematics_base.h>
#include <moveit/robot_model/robot_model.h>

#include <dobot_msgs_v4/srv/inverse_kin.hpp>
#include <dobot_msgs_v4/srv/positive_kin.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace dobot_kinematics_plugin {

class DobotKinematicsPlugin : public kinematics::KinematicsBase
{
public:
  DobotKinematicsPlugin() = default;
  // 显式析构：先释放 KDL 子插件再释放 ClassLoader，吞掉 pluginlib 卸载异常。
  ~DobotKinematicsPlugin() override;

  // —— 非纯虚，但必须重写以建服务客户端 / 加载 KDL 子插件 / 读参数 ——
  bool initialize(const rclcpp::Node::SharedPtr& node,
                 const moveit::core::RobotModel& robot_model,
                 const std::string& group_name,
                 const std::string& base_frame,
                 const std::vector<std::string>& tip_frames,
                 double search_discretization) override;

  // —— 纯虚：FK ——
  bool getPositionFK(const std::vector<std::string>& link_names,
                     const std::vector<double>& joint_angles,
                     std::vector<geometry_msgs::msg::Pose>& poses) const override;

  // —— 纯虚：单解 IK ——
  bool getPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                     const std::vector<double>& ik_seed_state,
                     std::vector<double>& solution,
                     moveit_msgs::msg::MoveItErrorCodes& error_code,
                     const kinematics::KinematicsQueryOptions& options =
                         kinematics::KinematicsQueryOptions()) const override;

  // —— searchPositionIK 各重载（Humble 纯虚 + 带 callback 的虚函数）——
  bool searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                        const std::vector<double>& ik_seed_state,
                        double timeout,
                        std::vector<double>& solution,
                        moveit_msgs::msg::MoveItErrorCodes& error_code,
                        const kinematics::KinematicsQueryOptions& options =
                            kinematics::KinematicsQueryOptions()) const override;

  bool searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                        const std::vector<double>& ik_seed_state,
                        double timeout,
                        const std::vector<double>& consistency_limits,
                        std::vector<double>& solution,
                        moveit_msgs::msg::MoveItErrorCodes& error_code,
                        const kinematics::KinematicsQueryOptions& options =
                            kinematics::KinematicsQueryOptions()) const override;

  bool searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                        const std::vector<double>& ik_seed_state,
                        double timeout,
                        std::vector<double>& solution,
                        const IKCallbackFn& solution_callback,
                        moveit_msgs::msg::MoveItErrorCodes& error_code,
                        const kinematics::KinematicsQueryOptions& options =
                            kinematics::KinematicsQueryOptions()) const override;

  bool searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                        const std::vector<double>& ik_seed_state,
                        double timeout,
                        const std::vector<double>& consistency_limits,
                        std::vector<double>& solution,
                        const IKCallbackFn& solution_callback,
                        moveit_msgs::msg::MoveItErrorCodes& error_code,
                        const kinematics::KinematicsQueryOptions& options =
                            kinematics::KinematicsQueryOptions()) const override;

  // —— 纯虚：关节/链接名 ——
  const std::vector<std::string>& getJointNames() const override;
  const std::vector<std::string>& getLinkNames() const override;

private:
  // 4 个 searchPositionIK 重载统一归一化到这里。
  bool solveIK(const geometry_msgs::msg::Pose& ik_pose,
               const std::vector<double>& ik_seed_state,
               double timeout,
               const std::vector<double>* consistency_limits,
               const IKCallbackFn* solution_callback,
               std::vector<double>& solution,
               moveit_msgs::msg::MoveItErrorCodes& error_code,
               const kinematics::KinematicsQueryOptions& options) const;

  // 调控制器 FK/IK，失败返回 false。
  bool callControllerFK(const std::vector<double>& joint_rad,
                        geometry_msgs::msg::Pose& pose_out) const;
  bool callControllerIK(const geometry_msgs::msg::Pose& pose,
                        const std::vector<double>& seed_rad,
                        std::vector<double>& solution_rad) const;

  // KDL 兜底。
  bool kdlFK(const std::vector<std::string>& link_names,
             const std::vector<double>& joint_angles,
             std::vector<geometry_msgs::msg::Pose>& poses) const;
  bool kdlIK(const geometry_msgs::msg::Pose& ik_pose,
             const std::vector<double>& ik_seed_state,
             double timeout,
             const std::vector<double>* consistency_limits,
             const IKCallbackFn* solution_callback,
             std::vector<double>& solution,
             moveit_msgs::msg::MoveItErrorCodes& error_code,
             const kinematics::KinematicsQueryOptions& options) const;

  // ROS 服务客户端
  // 注意: client 必须用独立辅助 Node 创建, 不能用 move_group 的 node。
  // move_group 的 node 已加入 MultiThreadedExecutor, 插件在回调线程里同步调服务时,
  // 响应回调需要被 spin 才能处理, 但 move_group 的 executor 线程被阻塞了。
  // 辅助 Node 有自己的 executor + 独立 spin 线程, 保证响应能被处理。
  rclcpp::Node::SharedPtr helper_node_;
  std::shared_ptr<rclcpp::executors::SingleThreadedExecutor> helper_exec_;
  std::thread helper_spin_thread_;
  std::atomic<bool> helper_spin_running_{ false };
  rclcpp::Client<dobot_msgs_v4::srv::PositiveKin>::SharedPtr fk_client_;
  rclcpp::Client<dobot_msgs_v4::srv::InverseKin>::SharedPtr ik_client_;

  // KDL 子插件（兜底）。
  // 用 createUniqueInstance 而非 createSharedInstance，避免退出时析构死锁。
  // loader 必须存活到插件析构完成（作为成员保证）。
  std::shared_ptr<pluginlib::ClassLoader<kinematics::KinematicsBase>> kdl_loader_;
  pluginlib::UniquePtr<kinematics::KinematicsBase> kdl_plugin_;

  // —— 来自 kinematics.yaml 的配置 ——
  std::string fk_service_name_{ "/dobot_bringup_ros2/srv/PositiveKin" };
  std::string ik_service_name_{ "/dobot_bringup_ros2/srv/InverseKin" };
  double service_timeout_sec_{ 1.0 };
  bool use_kdl_fallback_{ true };
  // 控制器是否启用：initialize 时由 use_controller 参数 + 服务就绪探测决定。
  // mutable 允许 const 方法（callControllerIK/FK）在恢复重连时修改此标志。
  mutable bool controller_enabled_{ true };

  // 单位/姿态换算
  double position_scale_{ 0.001 };          // 控制器->MoveIt: mm->m
  double angle_scale_{ M_PI / 180.0 };       // 控制器->MoveIt: deg->rad
  enum class OrientConv
  {
    ROTATION_VECTOR,
    RPY
  } orient_conv_{ OrientConv::ROTATION_VECTOR };

  std::string user_frame_{ "0" };
  std::string tool_frame_{ "0" };

  // 关节/链接名缓存（initialize 中从 JointModelGroup 获取）
  std::vector<std::string> joint_names_;
  std::vector<std::string> link_names_;
};

}  // namespace dobot_kinematics_plugin
