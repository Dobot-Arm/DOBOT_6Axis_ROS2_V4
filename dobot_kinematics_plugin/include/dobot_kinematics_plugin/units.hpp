// Copyright (c) Dobot. BSD License.
// 控制器(MM/度) <-> MoveIt(米/弧度/四元数) 之间的单位与姿态换算。
// 默认姿态约定：rx,ry,rz 为旋转向量(轴角)，幅度为旋转角，单位度。
//   依据：Dobot CR 仪表盘 API 标准；与 command.cpp:53 deg2Rad 的度数约定一致。
#pragma once

#include <Eigen/Geometry>
#include <array>
#include <cmath>

namespace dobot_kinematics_plugin {

constexpr double kDeg2Rad = M_PI / 180.0;
constexpr double kRad2Deg = 180.0 / M_PI;

// 旋转向量(度) -> 四元数。r=(rx,ry,rz)，模长=角度(度)，方向=转轴。
inline Eigen::Quaterniond rotVecDegToQuat(double rx, double ry, double rz)
{
  Eigen::Vector3d r(rx, ry, rz);
  double norm_deg = r.norm();
  if (norm_deg < 1e-9)
    return Eigen::Quaterniond::Identity();
  double angle_rad = norm_deg * kDeg2Rad;
  Eigen::Vector3d axis = r / norm_deg;
  return Eigen::Quaterniond(Eigen::AngleAxisd(angle_rad, axis));
}

// 四元数 -> 旋转向量(度)。
inline std::array<double, 3> quatToRotVecDeg(const Eigen::Quaterniond& q)
{
  // 规范化以避免 AngleAxis 数值不稳定
  Eigen::Quaterniond qn = q.normalized();
  Eigen::AngleAxisd aa(qn);
  if (aa.angle() < 1e-9)
    return { 0.0, 0.0, 0.0 };
  double ang_deg = aa.angle() * kRad2Deg;
  Eigen::Vector3d rv = aa.axis() * ang_deg;
  return { rv.x(), rv.y(), rv.z() };
}

// RPY(度, ROS 固有顺序 ZYX 即 setRPY) -> 四元数。
// 注意：仅当 kinematics.yaml 中 orientation_convention=rpy 时使用；
//       控制器的 RPY 顺序需运行时确认，此处采用与 tf2 一致的 ZYX intrinsic。
inline Eigen::Quaterniond rpyDegToQuat(double r_deg, double p_deg, double y_deg)
{
  double r = r_deg * kDeg2Rad, p = p_deg * kDeg2Rad, y = y_deg * kDeg2Rad;
  // 与 tf2::Quaternion::setRPY(r, p, y) 一致
  Eigen::Quaterniond q = Eigen::AngleAxisd(y, Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(p, Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(r, Eigen::Vector3d::UnitX());
  return q;
}

// 四元数 -> RPY(度, ZYX)。
inline std::array<double, 3> quatToRpyDeg(const Eigen::Quaterniond& q)
{
  Eigen::Vector3d rpy = q.toRotationMatrix().eulerAngles(2, 1, 0);  // ZYX -> (y,p,r)
  double r = rpy.z(), p = rpy.y(), y = rpy.x();
  return { r * kRad2Deg, p * kRad2Deg, y * kRad2Deg };
}

// 把弧度角度归一化到 [-pi, pi]，避免控制器返回超 ±pi 的关节角。
inline double normalizeAngle(double rad)
{
  double v = std::fmod(rad + M_PI, 2.0 * M_PI);
  if (v < 0)
    v += 2.0 * M_PI;
  return v - M_PI;
}

}  // namespace dobot_kinematics_plugin
