// Copyright (c) Dobot. BSD License.
// 解析 Dobot 控制器 ROS 服务返回的 "{a,b,c,d,e,f}" 字符串。
// 依据：dobot_bringup_v4/src/command.cpp 中 doTcpCmd_f 解析逻辑，
//       robot_return 形如 "{x,y,z,rx,ry,rz}" 或 "{j1,j2,j3,j4,j5,j6}"，带花括号。
#pragma once

#include <array>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

namespace dobot_kinematics_plugin {

// 把 "{1.0, 2.0, 3.0, 4.0, 5.0, 6.0}" 解析为 std::array<double,6>。
// 容错：去除空白与 '{}'，按 ',' 分割，逐段 std::stod。
// 返回 false 表示格式不符（长度不等于 6 或解析失败）。
inline bool parseBraceList6(const std::string& s, std::array<double, 6>& out)
{
  std::string t;
  t.reserve(s.size());
  for (char c : s)
  {
    if (std::isspace(static_cast<unsigned char>(c)) || c == '{' || c == '}' || c == '[' || c == ']' ||
        c == '(' || c == ')')
      continue;
    t += c;
  }

  std::vector<std::string> parts;
  parts.reserve(6);
  std::stringstream ss(t);
  std::string item;
  while (std::getline(ss, item, ','))
  {
    if (!item.empty())
      parts.push_back(item);
  }
  if (parts.size() != 6)
    return false;

  try
  {
    for (std::size_t i = 0; i < 6; ++i)
      out[i] = std::stod(parts[i]);
  }
  catch (...)
  {
    return false;
  }
  return true;
}

}  // namespace dobot_kinematics_plugin
