// Copyright (c) Dobot. BSD License.
// DobotKinematicsPlugin 实现：FK/IK 委托控制器 ROS 服务，KDL 兜底。
//
// 服务接口核对依据：
//   dobot_bringup_v4/src/cr_robot_ros2.cpp:193-194（服务注册）
//   dobot_bringup_v4/src/parseTool.cpp:208-229（请求字符串构造）
//   dobot_bringup_v4/src/command.cpp:53,164-213（单位换算与返回字符串解析）
//   dobot_msgs_v4/srv/{Positive,Inverse}Kin.srv
#include "dobot_kinematics_plugin/dobot_kinematics_plugin.hpp"

#include "dobot_kinematics_plugin/string_parser.hpp"
#include "dobot_kinematics_plugin/units.hpp"

#include <moveit/robot_model/joint_model_group.h>
#include <pluginlib/class_list_macros.hpp>
#include <Eigen/Geometry>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <sstream>
#include <thread>

namespace dobot_kinematics_plugin {
namespace {

// 健壮读取 kinematics.yaml 中 group 下的自定义参数。
// MoveIt 把 kinematics.yaml 加载到 robot_description_kinematics.<group>.<key> 下，
// move_group 节点通常 allow_undeclared_parameters=true，故这些参数已被隐式声明。
// 这里用 declare(带默认)+ catch already-declared + get 的组合，兼容所有情况。
template <typename T>
T readParam(const rclcpp::Node::SharedPtr& node, const std::string& group,
            const std::string& key, const T& default_val)
{
  std::vector<std::string> names = {
    "robot_description_kinematics." + group + "." + key,
    "~" + key,
    key
  };
  for (const auto& name : names)
  {
    try
    {
      node->declare_parameter(name, rclcpp::ParameterValue(default_val));
    }
    catch (const rclcpp::exceptions::ParameterAlreadyDeclaredException&)
    {
      // 已声明（通常是 MoveIt 加载 kinematics.yaml 时设置的），下面 get 即可
    }
    catch (...)
    {
      continue;
    }
    T val = default_val;
    if (node->get_parameter(name, val))
      return val;
  }
  return default_val;
}

// 把 6 个弧度关节角格式化成控制器期望的 "{deg,...,deg}" 字符串（用于 joint_near）。
std::string formatJointNearDeg(const std::vector<double>& joint_rad, double angle_scale)
{
  std::ostringstream ss;
  ss << "{";
  for (std::size_t i = 0; i < joint_rad.size(); ++i)
  {
    if (i)
      ss << ",";
    ss << (joint_rad[i] / angle_scale);  // rad -> deg
  }
  ss << "}";
  return ss.str();
}

// 服务健康检查 + 断连自动重连。
// 首次失败时尝试 wait_for_service 恢复（2s 阻塞），成功则恢复 controller_enabled_。
// 用 WARN_ONCE 避免日志风暴，切换状态时打 INFO。
bool ensureServiceReady(rclcpp::ClientBase::SharedPtr client,
                         const rclcpp::Logger& logger,
                         const std::string& name,
                         bool& controller_enabled)
{
  if (client->service_is_ready())
    return true;

  // 服务不可用 → 尝试重连
  if (!controller_enabled)
  {
    // 已标记不可用，尝试恢复
    RCLCPP_INFO_ONCE(logger, "Attempting reconnection to '%s'...", name.c_str());
    if (client->wait_for_service(std::chrono::seconds(2)))
    {
      controller_enabled = true;
      RCLCPP_INFO(logger, "Reconnected to '%s', controller path restored", name.c_str());
      return true;
    }
    return false;
  }

  // 首次检测到断连
  RCLCPP_WARN(logger, "Service '%s' lost, switching to KDL fallback", name.c_str());
  controller_enabled = false;
  return false;
}

}  // namespace

// ============================================================================
// initialize
// ============================================================================
bool DobotKinematicsPlugin::initialize(const rclcpp::Node::SharedPtr& node,
                                        const moveit::core::RobotModel& robot_model,
                                        const std::string& group_name,
                                        const std::string& base_frame,
                                        const std::vector<std::string>& tip_frames,
                                        double search_discretization)
{
  // 1. 保存 node（基类 storeValues 不填 node_）
  node_ = node;
  //    填充基类 protected 成员（不用基类 initialize，其默认实现会打 deprecated 警告并返回 false）
  storeValues(robot_model, group_name, base_frame, tip_frames, search_discretization);

  const rclcpp::Logger& logger = node->get_logger();

  // 2. 读取 kinematics.yaml 自定义参数
  fk_service_name_ = readParam<std::string>(node, group_name, "fk_service_name", fk_service_name_);
  ik_service_name_ = readParam<std::string>(node, group_name, "ik_service_name", ik_service_name_);
  service_timeout_sec_ = readParam<double>(node, group_name, "service_timeout", service_timeout_sec_);
  use_kdl_fallback_ = readParam<bool>(node, group_name, "use_kdl_fallback", use_kdl_fallback_);
  user_frame_ = readParam<std::string>(node, group_name, "user_frame", user_frame_);
  tool_frame_ = readParam<std::string>(node, group_name, "tool_frame", tool_frame_);

  std::string pos_unit = readParam<std::string>(node, group_name, "position_unit", std::string("mm"));
  std::string ang_unit = readParam<std::string>(node, group_name, "angle_unit", std::string("deg"));
  std::string orient = readParam<std::string>(node, group_name, "orientation_convention",
                                                std::string("rotation_vector"));
  position_scale_ = (pos_unit == "mm") ? 0.001 : 1.0;
  angle_scale_ = (ang_unit == "deg") ? (M_PI / 180.0) : 1.0;
  orient_conv_ = (orient == "rpy") ? OrientConv::RPY : OrientConv::ROTATION_VECTOR;

  RCLCPP_INFO(logger,
              "DobotKinematicsPlugin: group='%s' base='%s' tip[0]='%s' "
              "fk='%s' ik='%s' timeout=%.3fs fallback=%s pos_unit=%s ang_unit=%s orient=%s",
              group_name.c_str(), base_frame.c_str(),
              tip_frames.empty() ? "" : tip_frames.front().c_str(),
              fk_service_name_.c_str(), ik_service_name_.c_str(), service_timeout_sec_,
              use_kdl_fallback_ ? "on" : "off", pos_unit.c_str(), ang_unit.c_str(), orient.c_str());

  // 3. 创建辅助 Node + 独立 spin 线程, 用它创建服务 client。
  //    不能用 move_group 的 node: 它已在 MultiThreadedExecutor 中, 插件同步调服务时
  //    响应回调没人 spin 会超时; 也不能临时 add_node 到另一个 executor (会抛异常)。
  helper_node_ = std::make_shared<rclcpp::Node>("dobot_kin_helper");
  helper_exec_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
  helper_exec_->add_node(helper_node_);
  helper_spin_running_.store(true);
  helper_spin_thread_ = std::thread([this]() {
    while (helper_spin_running_.load() && rclcpp::ok())
      helper_exec_->spin_once(std::chrono::milliseconds(2));
  });
  fk_client_ = helper_node_->create_client<dobot_msgs_v4::srv::PositiveKin>(fk_service_name_);
  ik_client_ = helper_node_->create_client<dobot_msgs_v4::srv::InverseKin>(ik_service_name_);

  // 3a. 是否启用控制器正逆解，由 launch 通过 use_controller 参数决定：
  //     - use_dobot_solver:=false（默认/虚拟演示）：不传 use_controller -> KDL
  //     - use_dobot_solver:=true（真机）：传 use_controller:=true -> 启用控制器
  //     仅当 use_controller=true 且 FK/IK 服务都就绪时才真正启用控制器路径。
  const bool use_controller = readParam<bool>(node, group_name, "use_controller", false);
  if (!use_controller)
  {
    controller_enabled_ = false;
    RCLCPP_INFO(logger, "use_controller=false, 使用 KDL 正逆解");
  }
  else
  {
    // 等 helper_node discovery 完成 (service_is_ready 非阻塞, 刚创建时可能 false)
    const bool fk_ready = fk_client_->wait_for_service(std::chrono::seconds(2));
    const bool ik_ready = ik_client_->wait_for_service(std::chrono::seconds(2));
    controller_enabled_ = fk_ready && ik_ready;
    if (controller_enabled_)
    {
      RCLCPP_INFO(logger, "use_controller=true 且服务就绪 (fk=%d ik=%d)", fk_ready, ik_ready);
    }
    else
    {
      RCLCPP_WARN(logger, "use_controller=true 但服务未就绪 (fk=%d ik=%d), 退回 KDL", fk_ready, ik_ready);
    }
  }

  // 4. 缓存关节名 / 链接名
  const moveit::core::JointModelGroup* jmg = robot_model.getJointModelGroup(group_name);
  if (!jmg)
  {
    RCLCPP_ERROR(logger, "JointModelGroup '%s' not found in RobotModel", group_name.c_str());
    return false;
  }
  joint_names_ = jmg->getActiveJointModelNames();
  link_names_ = jmg->getLinkModelNames();

  // 5. 加载 KDL 子插件作为兜底（用同一 RobotModel/Group，保证退化时模型一致）
  //    用 createUniqueInstance 而非 createSharedInstance，避免退出时析构死锁。
  if (use_kdl_fallback_)
  {
    try
    {
      kdl_loader_ = std::make_shared<pluginlib::ClassLoader<kinematics::KinematicsBase>>(
          "moveit_core", "kinematics::KinematicsBase");
      kdl_plugin_ = kdl_loader_->createUniqueInstance("kdl_kinematics_plugin/KDLKinematicsPlugin");
      bool ok = kdl_plugin_->initialize(node, robot_model, group_name, base_frame,
                                         tip_frames, search_discretization);
      if (!ok)
      {
        RCLCPP_ERROR(logger, "Failed to initialize KDL fallback plugin");
        kdl_plugin_.reset();
      }
      else
      {
        RCLCPP_INFO(logger, "KDL fallback plugin loaded successfully");
      }
    }
    catch (const pluginlib::PluginlibException& e)
    {
      RCLCPP_ERROR(logger, "Failed to load KDL fallback plugin: %s", e.what());
    }
  }

  return true;
}

// ============================================================================
// FK
// ============================================================================
bool DobotKinematicsPlugin::getPositionFK(const std::vector<std::string>& link_names,
                                            const std::vector<double>& joint_angles,
                                            std::vector<geometry_msgs::msg::Pose>& poses) const
{
  poses.clear();
  // FK 始终走 KDL：执行轨迹时 trajectory_execution_manager / planning_scene_monitor 会
  // 高频调用 FK 做状态监测与碰撞检测，若走控制器服务（网络往返 + 同步阻塞）会拖慢
  // move_group 主线程导致执行卡顿。FK 在执行阶段用理论 DH 完全够用，标定 DH 只在
  // 目标位姿 IK 查询（getPositionIK）时才需要精度。
  if (use_kdl_fallback_ && kdl_plugin_)
    return kdlFK(link_names, joint_angles, poses);
  return false;
}

bool DobotKinematicsPlugin::callControllerFK(const std::vector<double>& joint_rad,
                                              geometry_msgs::msg::Pose& pose_out) const
{
  if (!controller_enabled_ || !fk_client_ || joint_rad.size() < 6)
    return false;

  const rclcpp::Logger& logger = node_->get_logger();
  if (!ensureServiceReady(fk_client_, logger, fk_service_name_, controller_enabled_))
    return false;

  // 请求：关节角 rad -> deg
  auto req = std::make_shared<dobot_msgs_v4::srv::PositiveKin::Request>();
  req->j1 = joint_rad[0] / angle_scale_;
  req->j2 = joint_rad[1] / angle_scale_;
  req->j3 = joint_rad[2] / angle_scale_;
  req->j4 = joint_rad[3] / angle_scale_;
  req->j5 = joint_rad[4] / angle_scale_;
  req->j6 = joint_rad[5] / angle_scale_;
  req->user = user_frame_;
  req->tool = tool_frame_;

  auto future = fk_client_->async_send_request(req);
  if (future.wait_for(std::chrono::duration<double>(service_timeout_sec_)) != std::future_status::ready)
  {
    RCLCPP_WARN(logger, "FK service call timed out (%.3fs)", service_timeout_sec_);
    return false;
  }
  auto resp = future.get();
  if (!resp || resp->res != 0)
  {
    RCLCPP_WARN(logger, "FK service returned res=%d robot_return='%s'",
                resp ? resp->res : -1, resp ? resp->robot_return.c_str() : "");
    return false;
  }

  std::array<double, 6> v{};
  if (!parseBraceList6(resp->robot_return, v))
  {
    RCLCPP_WARN(logger, "FK response parse failed: '%s'", resp->robot_return.c_str());
    return false;
  }

  // 位置 mm -> m
  pose_out.position.x = v[0] * position_scale_;
  pose_out.position.y = v[1] * position_scale_;
  pose_out.position.z = v[2] * position_scale_;

  // 姿态 deg -> 四元数
  Eigen::Quaterniond q;
  if (orient_conv_ == OrientConv::RPY)
    q = rpyDegToQuat(v[3], v[4], v[5]);
  else
    q = rotVecDegToQuat(v[3], v[4], v[5]);
  pose_out.orientation.x = q.x();
  pose_out.orientation.y = q.y();
  pose_out.orientation.z = q.z();
  pose_out.orientation.w = q.w();
  return true;
}

bool DobotKinematicsPlugin::kdlFK(const std::vector<std::string>& link_names,
                                   const std::vector<double>& joint_angles,
                                   std::vector<geometry_msgs::msg::Pose>& poses) const
{
  if (!kdl_plugin_)
    return false;
  return kdl_plugin_->getPositionFK(link_names, joint_angles, poses);
}

// ============================================================================
// IK（4 个 searchPositionIK 重载 + getPositionIK）
//
// 分层策略：
//   - searchPositionIK：OMPL 笛卡尔约束规划用 → 直接走 KDL（快速）。
//     精准场景下推荐以关节约束方式发送目标（脚本先调控制器IK算关节角），
//     OMPL纯关节空间规划无需调searchPositionIK
//   - getPositionIK：单点查询 → solveIK (控制器优先→KDL兜底)
//   - getPositionFK：始终走 KDL
// ============================================================================

// —— searchPositionIK：始终走 KDL ——
bool DobotKinematicsPlugin::searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                                              const std::vector<double>& ik_seed_state,
                                              double timeout,
                                              std::vector<double>& solution,
                                              moveit_msgs::msg::MoveItErrorCodes& error_code,
                                              const kinematics::KinematicsQueryOptions& options) const
{
  if (controller_enabled_)
    return solveIK(ik_pose, ik_seed_state, timeout, nullptr, nullptr, solution, error_code, options);
  return kdlIK(ik_pose, ik_seed_state, timeout, nullptr, nullptr, solution, error_code, options);
}

bool DobotKinematicsPlugin::searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                                              const std::vector<double>& ik_seed_state,
                                              double timeout,
                                              const std::vector<double>& consistency_limits,
                                              std::vector<double>& solution,
                                              moveit_msgs::msg::MoveItErrorCodes& error_code,
                                              const kinematics::KinematicsQueryOptions& options) const
{
  if (controller_enabled_)
    return solveIK(ik_pose, ik_seed_state, timeout, &consistency_limits, nullptr, solution, error_code, options);
  return kdlIK(ik_pose, ik_seed_state, timeout, &consistency_limits, nullptr, solution, error_code, options);
}

bool DobotKinematicsPlugin::searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                                              const std::vector<double>& ik_seed_state,
                                              double timeout,
                                              std::vector<double>& solution,
                                              const IKCallbackFn& solution_callback,
                                              moveit_msgs::msg::MoveItErrorCodes& error_code,
                                              const kinematics::KinematicsQueryOptions& options) const
{
  if (controller_enabled_)
    return solveIK(ik_pose, ik_seed_state, timeout, nullptr, &solution_callback, solution, error_code, options);
  return kdlIK(ik_pose, ik_seed_state, timeout, nullptr, &solution_callback, solution, error_code, options);
}

bool DobotKinematicsPlugin::searchPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                                              const std::vector<double>& ik_seed_state,
                                              double timeout,
                                              const std::vector<double>& consistency_limits,
                                              std::vector<double>& solution,
                                              const IKCallbackFn& solution_callback,
                                              moveit_msgs::msg::MoveItErrorCodes& error_code,
                                              const kinematics::KinematicsQueryOptions& options) const
{
  if (controller_enabled_)
    return solveIK(ik_pose, ik_seed_state, timeout, &consistency_limits, &solution_callback, solution,
                   error_code, options);
  return kdlIK(ik_pose, ik_seed_state, timeout, &consistency_limits, &solution_callback, solution,
               error_code, options);
}

// —— getPositionIK：单点查询，use_controller=true 时走控制器（标定 DH），否则 KDL ——
bool DobotKinematicsPlugin::getPositionIK(const geometry_msgs::msg::Pose& ik_pose,
                                           const std::vector<double>& ik_seed_state,
                                           std::vector<double>& solution,
                                           moveit_msgs::msg::MoveItErrorCodes& error_code,
                                           const kinematics::KinematicsQueryOptions& options) const
{
  return solveIK(ik_pose, ik_seed_state, default_timeout_, nullptr, nullptr, solution, error_code, options);
}

bool DobotKinematicsPlugin::solveIK(const geometry_msgs::msg::Pose& ik_pose,
                                      const std::vector<double>& ik_seed_state,
                                      double timeout,
                                      const std::vector<double>* consistency_limits,
                                      const IKCallbackFn* solution_callback,
                                      std::vector<double>& solution,
                                      moveit_msgs::msg::MoveItErrorCodes& error_code,
                                      const kinematics::KinematicsQueryOptions& /*options*/) const
{
  // 1. 优先控制器 IK
  std::vector<double> sol;
  if (callControllerIK(ik_pose, ik_seed_state, sol))
  {
    bool accept = true;
    // 1a. 一致性校验
    if (consistency_limits && consistency_limits->size() == sol.size())
    {
      for (std::size_t i = 0; i < sol.size(); ++i)
      {
        if (std::abs(sol[i] - ik_seed_state[i]) > (*consistency_limits)[i])
        {
          accept = false;
          break;
        }
      }
    }
    // 1b. 碰撞/有效性回调
    if (accept && solution_callback)
    {
      moveit_msgs::msg::MoveItErrorCodes cb_err;
      (*solution_callback)(ik_pose, sol, cb_err);
      if (cb_err.val != moveit_msgs::msg::MoveItErrorCodes::SUCCESS)
        accept = false;
    }
    if (accept)
    {
      solution = sol;
      error_code.val = moveit_msgs::msg::MoveItErrorCodes::SUCCESS;
      return true;
    }
  }

  // 2. 控制器失败或不被接受 -> KDL 兜底
  if (use_kdl_fallback_ && kdl_plugin_)
  {
    RCLCPP_WARN_ONCE(node_->get_logger(), "Controller IK failed, falling back to KDL");
    return kdlIK(ik_pose, ik_seed_state, timeout, consistency_limits, solution_callback,
                  solution, error_code, kinematics::KinematicsQueryOptions());
  }

  error_code.val = moveit_msgs::msg::MoveItErrorCodes::NO_IK_SOLUTION;
  return false;
}

bool DobotKinematicsPlugin::callControllerIK(const geometry_msgs::msg::Pose& pose,
                                              const std::vector<double>& seed_rad,
                                              std::vector<double>& solution_rad) const
{
  if (!controller_enabled_ || !ik_client_ || seed_rad.size() < 6)
    return false;

  const rclcpp::Logger& logger = node_->get_logger();
  if (!ensureServiceReady(ik_client_, logger, ik_service_name_, controller_enabled_))
    return false;

  // 位置 m -> mm；姿态 四元数 -> 旋转向量(deg) 或 RPY(deg)
  auto req = std::make_shared<dobot_msgs_v4::srv::InverseKin::Request>();
  req->x = pose.position.x / position_scale_;
  req->y = pose.position.y / position_scale_;
  req->z = pose.position.z / position_scale_;

  Eigen::Quaterniond q(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);
  std::array<double, 3> orient_deg;
  if (orient_conv_ == OrientConv::RPY)
    orient_deg = quatToRpyDeg(q);
  else
    orient_deg = quatToRotVecDeg(q);
  req->rx = orient_deg[0];
  req->ry = orient_deg[1];
  req->rz = orient_deg[2];

  // 用 seed 作为关节接近参考，保证解的连续性
  req->use_joint_near = "1";
  req->joint_near = formatJointNearDeg(seed_rad, angle_scale_);
  req->user = user_frame_;
  req->tool = tool_frame_;

  auto future = ik_client_->async_send_request(req);
  // helper_node_ 有独立 spin 线程, 响应会被处理, 直接 wait_for 即可。
  if (future.wait_for(std::chrono::duration<double>(service_timeout_sec_)) != std::future_status::ready)
  {
    RCLCPP_WARN(logger, "IK service call timed out (%.3fs)", service_timeout_sec_);
    return false;
  }
  auto resp = future.get();
  if (!resp || resp->res != 0)
  {
    RCLCPP_WARN(logger, "IK service returned res=%d robot_return='%s'",
                resp ? resp->res : -1, resp ? resp->robot_return.c_str() : "");
    return false;
  }

  std::array<double, 6> v{};
  if (!parseBraceList6(resp->robot_return, v))
  {
    RCLCPP_WARN(logger, "IK response parse failed: '%s'", resp->robot_return.c_str());
    return false;
  }

  // 关节角 deg -> rad，并归一化到 [-pi, pi]
  solution_rad.resize(6);
  for (int i = 0; i < 6; ++i)
    solution_rad[i] = normalizeAngle(v[i] * angle_scale_);
  return true;
}

bool DobotKinematicsPlugin::kdlIK(const geometry_msgs::msg::Pose& ik_pose,
                                   const std::vector<double>& ik_seed_state,
                                   double timeout,
                                   const std::vector<double>* consistency_limits,
                                   const IKCallbackFn* solution_callback,
                                   std::vector<double>& solution,
                                   moveit_msgs::msg::MoveItErrorCodes& error_code,
                                   const kinematics::KinematicsQueryOptions& options) const
{
  if (!kdl_plugin_)
  {
    error_code.val = moveit_msgs::msg::MoveItErrorCodes::NO_IK_SOLUTION;
    return false;
  }
  if (consistency_limits && solution_callback)
    return kdl_plugin_->searchPositionIK(ik_pose, ik_seed_state, timeout, *consistency_limits, solution,
                                          *solution_callback, error_code, options);
  if (consistency_limits)
    return kdl_plugin_->searchPositionIK(ik_pose, ik_seed_state, timeout, *consistency_limits, solution,
                                          error_code, options);
  if (solution_callback)
    return kdl_plugin_->searchPositionIK(ik_pose, ik_seed_state, timeout, solution, *solution_callback,
                                          error_code, options);
  return kdl_plugin_->searchPositionIK(ik_pose, ik_seed_state, timeout, solution, error_code, options);
}

// ============================================================================
// getJointNames / getLinkNames
// ============================================================================
const std::vector<std::string>& DobotKinematicsPlugin::getJointNames() const
{
  return joint_names_;
}

const std::vector<std::string>& DobotKinematicsPlugin::getLinkNames() const
{
  return link_names_;
}

// ============================================================================
// 析构：保证 kdl_plugin_ 先于 kdl_loader_ 释放，吞掉 pluginlib 卸载异常，
//       避免 move_group/rviz 退出时死锁、ctrl+c 无效。
// ============================================================================
DobotKinematicsPlugin::~DobotKinematicsPlugin()
{
  // 先停止辅助 spin 线程
  helper_spin_running_.store(false);
  if (helper_spin_thread_.joinable())
    helper_spin_thread_.join();
  try
  {
    kdl_plugin_.reset();   // 先释放插件实例
  }
  catch (...)
  {
  }
  try
  {
    kdl_loader_.reset();   // 再释放 ClassLoader
  }
  catch (...)
  {
  }
}

}  // namespace dobot_kinematics_plugin

// pluginlib 导出
PLUGINLIB_EXPORT_CLASS(dobot_kinematics_plugin::DobotKinematicsPlugin, kinematics::KinematicsBase)
