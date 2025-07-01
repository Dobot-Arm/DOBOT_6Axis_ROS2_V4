/**
 ***********************************************************************************************************************
 *
 * @author qiufengwu FTX
 * @date   2024/01/23
 * @date   2025/02/27
 *
 ***********************************************************************************************************************
 */

#ifndef CRROBOTROS2_H
#define CRROBOTROS2_H

#include <rclcpp/rclcpp.hpp>
#include <rclcpp/node.hpp>
#include <rclcpp/executor.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <memory>
#include <dobot_bringup/parseTool.h>
#include <rclcpp_action/rclcpp_action.hpp>
#include <rclcpp_action/create_server.hpp>
#include <std_msgs/msg/string.hpp>
#include <dobot_bringup/command.h>
#include <sensor_msgs/msg/joint_state.hpp>
#include <dobot_msgs_v4/srv/enable_robot.hpp>
#include <dobot_msgs_v4/srv/disable_robot.hpp>
#include <dobot_msgs_v4/srv/clear_error.hpp>
#include <dobot_msgs_v4/srv/speed_factor.hpp>
#include <dobot_msgs_v4/srv/user.hpp>
#include <dobot_msgs_v4/srv/tool.hpp>
#include <dobot_msgs_v4/srv/robot_mode.hpp>
#include <dobot_msgs_v4/srv/set_payload.hpp>
#include <dobot_msgs_v4/srv/do.hpp>
#include <dobot_msgs_v4/srv/do_instant.hpp>
#include <dobot_msgs_v4/srv/tool_do.hpp>
#include <dobot_msgs_v4/srv/tool_do_instant.hpp>
#include <dobot_msgs_v4/srv/ao.hpp>
#include <dobot_msgs_v4/srv/ao_instant.hpp>
#include <dobot_msgs_v4/srv/acc_j.hpp>
#include <dobot_msgs_v4/srv/acc_l.hpp>
#include <dobot_msgs_v4/srv/vel_j.hpp>
#include <dobot_msgs_v4/srv/vel_l.hpp>
#include <dobot_msgs_v4/srv/cp.hpp>
#include <dobot_msgs_v4/srv/power_on.hpp>
#include <dobot_msgs_v4/srv/run_script.hpp>
#include <dobot_msgs_v4/srv/stop.hpp>
#include <dobot_msgs_v4/srv/pause.hpp>
#include <dobot_msgs_v4/srv/continue.hpp>
#include <dobot_msgs_v4/srv/set_collision_level.hpp>
#include <dobot_msgs_v4/srv/enable_safe_skin.hpp>
#include <dobot_msgs_v4/srv/set_safe_skin.hpp>
#include <dobot_msgs_v4/srv/start_path.hpp>
#include <dobot_msgs_v4/srv/get_start_pose.hpp>
#include <dobot_msgs_v4/srv/positive_kin.hpp>
#include <dobot_msgs_v4/srv/get_pose.hpp>
#include <dobot_msgs_v4/srv/inverse_kin.hpp>
#include <dobot_msgs_v4/srv/get_angle.hpp>
#include <dobot_msgs_v4/srv/emergency_stop.hpp>
#include <dobot_msgs_v4/srv/modbus_rtu_create.hpp>
#include <dobot_msgs_v4/srv/modbus_create.hpp>
#include <dobot_msgs_v4/srv/modbus_close.hpp>
#include <dobot_msgs_v4/srv/get_in_bits.hpp>
#include <dobot_msgs_v4/srv/get_in_regs.hpp>
#include <dobot_msgs_v4/srv/get_coils.hpp>
#include <dobot_msgs_v4/srv/set_coils.hpp>
#include <dobot_msgs_v4/srv/get_hold_regs.hpp>
#include <dobot_msgs_v4/srv/set_hold_regs.hpp>
#include <dobot_msgs_v4/srv/get_error_id.hpp>
#include <dobot_msgs_v4/srv/di.hpp>
#include <dobot_msgs_v4/srv/tool_di.hpp>
#include <dobot_msgs_v4/srv/ai.hpp>
#include <dobot_msgs_v4/srv/tool_ai.hpp>
#include <dobot_msgs_v4/srv/di_group.hpp>
#include <dobot_msgs_v4/srv/do_group.hpp>
#include <dobot_msgs_v4/srv/brake_control.hpp>
#include <dobot_msgs_v4/srv/start_drag.hpp>
#include <dobot_msgs_v4/srv/stop_drag.hpp>
#include <dobot_msgs_v4/srv/drag_sensivity.hpp>
#include <dobot_msgs_v4/srv/get_do.hpp>
#include <dobot_msgs_v4/srv/get_ao.hpp>
#include <dobot_msgs_v4/srv/get_do_group.hpp>
#include <dobot_msgs_v4/srv/set_tool485.hpp>
#include <dobot_msgs_v4/srv/set_safe_wall_enable.hpp>
#include <dobot_msgs_v4/srv/set_tool_power.hpp>
#include <dobot_msgs_v4/srv/set_tool_mode.hpp>
#include <dobot_msgs_v4/srv/set_back_distance.hpp>
#include <dobot_msgs_v4/srv/set_post_collision_mode.hpp>
#include <dobot_msgs_v4/srv/set_user.hpp>
#include <dobot_msgs_v4/srv/set_tool.hpp>
#include <dobot_msgs_v4/srv/calc_user.hpp>
#include <dobot_msgs_v4/srv/calc_tool.hpp>
#include <dobot_msgs_v4/srv/get_input_bool.hpp>
#include <dobot_msgs_v4/srv/get_input_int.hpp>
#include <dobot_msgs_v4/srv/get_input_float.hpp>
#include <dobot_msgs_v4/srv/get_output_bool.hpp>
#include <dobot_msgs_v4/srv/get_output_int.hpp>
#include <dobot_msgs_v4/srv/get_output_float.hpp>
#include <dobot_msgs_v4/srv/set_output_bool.hpp>
#include <dobot_msgs_v4/srv/set_output_int.hpp>
#include <dobot_msgs_v4/srv/set_output_float.hpp>
#include <dobot_msgs_v4/srv/mov_lio.hpp>
#include <dobot_msgs_v4/srv/mov_jio.hpp>
#include <dobot_msgs_v4/srv/arc.hpp>
#include <dobot_msgs_v4/srv/circle.hpp>
#include <dobot_msgs_v4/srv/move_jog.hpp>
#include <dobot_msgs_v4/srv/stop_move_jog.hpp>
#include <dobot_msgs_v4/srv/rel_mov_j_tool.hpp>
#include <dobot_msgs_v4/srv/rel_mov_l_tool.hpp>
#include <dobot_msgs_v4/srv/rel_mov_j_user.hpp>
#include <dobot_msgs_v4/srv/rel_mov_l_user.hpp>
#include <dobot_msgs_v4/srv/mov_j.hpp>
#include <dobot_msgs_v4/srv/mov_l.hpp>
#include <dobot_msgs_v4/srv/rel_joint_mov_j.hpp>
#include <dobot_msgs_v4/srv/get_current_command_id.hpp>
#include <dobot_msgs_v4/srv/servo_j.hpp>
#include <dobot_msgs_v4/srv/servo_p.hpp>

class CRRobotRos2 : public rclcpp::Node
{
public:
    CRRobotRos2();
    void init();
    static void execute_action(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request,
                               std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Response> response);
    void goalHandle();
    void backendTask();
    void getJointState(double *point);
    bool isEnable() const;
    bool isConnected() const;
    void getToolVectorActual(double *val);

protected:
    /**
     * TCP
     */
    bool EnableRobot(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Response> response);
    bool DisableRobot(const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Response> response);
    bool ClearError(const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Response> response);
    bool SpeedFactor(const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Response> response);
    bool User(const std::shared_ptr<dobot_msgs_v4::srv::User::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::User::Response> response);
    bool Tool(const std::shared_ptr<dobot_msgs_v4::srv::Tool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Tool::Response> response);
    bool RobotMode(const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Response> response);
    bool SetPayload(const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Response> response);
    bool DO(const std::shared_ptr<dobot_msgs_v4::srv::DO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DO::Response> response);
    bool DOInstant(const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Response> response);
    bool ToolDO(const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Response> response);
    bool ToolDOInstant(const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Response> response);
    bool AO(const std::shared_ptr<dobot_msgs_v4::srv::AO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AO::Response> response);
    bool AOInstant(const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Response> response);
    bool AccJ(const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Response> response);
    bool AccL(const std::shared_ptr<dobot_msgs_v4::srv::AccL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AccL::Response> response);
    bool VelJ(const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Response> response);
    bool VelL(const std::shared_ptr<dobot_msgs_v4::srv::VelL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::VelL::Response> response);
    bool CP(const std::shared_ptr<dobot_msgs_v4::srv::CP::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CP::Response> response);
    bool PowerOn(const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Response> response);
    bool RunScript(const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Response> response);
    bool Stop(const std::shared_ptr<dobot_msgs_v4::srv::Stop::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Stop::Response> response);
    bool Pause(const std::shared_ptr<dobot_msgs_v4::srv::Pause::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Pause::Response> response);
    bool Continue(const std::shared_ptr<dobot_msgs_v4::srv::Continue::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Continue::Response> response);
    bool EnableSafeSkin(const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Response> response);
    bool SetSafeSkin(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Response> response);
    bool GetStartPose(const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Response> response);
    bool StartPath(const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Response> response);
    bool PositiveKin(const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Response> response);
    bool InverseKin(const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Response> response);
    bool GetAngle(const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Response> response);
    bool GetPose(const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Response> response);
    bool EmergencyStop(const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Response> response);
    bool SetCollisionLevel(const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Response> response);
    bool ModbusRTUCreate(const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Response> response);
    bool ModbusCreate(const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Response> response);
    bool ModbusClose(const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Response> response);
    bool GetInBits(const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Response> response);
    bool GetInRegs(const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Response> response);
    bool GetCoils(const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Response> response);
    bool SetCoils(const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Response> response);
    bool GetHoldRegs(const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Response> response);
    bool SetHoldRegs(const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Response> response);
    bool GetErrorID(const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Response> response);
    bool DI(const std::shared_ptr<dobot_msgs_v4::srv::DI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DI::Response> response);
    bool ToolDI(const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Response> response);
    bool AI(const std::shared_ptr<dobot_msgs_v4::srv::AI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AI::Response> response);
    bool ToolAI(const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Response> response);
    bool DIGroup(const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Response> response);
    bool DOGroup(const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Response> response);
    bool BrakeControl(const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Response> response);
    bool StartDrag(const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Response> response);
    bool StopDrag(const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Response> response);
    bool DragSensivity(const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Response> response);
    bool GetDO(const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Response> response);
    bool GetAO(const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Response> response);
    bool GetDOGroup(const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Response> response);
    bool SetTool485(const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Response> response);
    bool SetSafeWallEnable(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Response> response);
    bool SetToolPower(const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Response> response);
    bool SetToolMode(const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Response> response);
    bool SetBackDistance(const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Response> response);
    bool SetPostCollisionMode(const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Response> response);
    bool SetUser(const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Response> response);
    bool SetTool(const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Response> response);
    bool CalcUser(const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Response> response);
    bool CalcTool(const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Response> response);
    bool GetInputBool(const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Response> response);
    bool GetInputInt(const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Response> response);
    bool GetInputFloat(const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Response> response);
    bool GetOutputBool(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Response> response);
    bool GetOutputInt(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Response> response);
    bool GetOutputFloat(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Response> response);
    bool SetOutputBool(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Response> response);
    bool SetOutputInt(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Response> response);
    bool SetOutputFloat(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Response> response);
    bool MovJ(const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Response> response);
    bool MovL(const std::shared_ptr<dobot_msgs_v4::srv::MovL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovL::Response> response);
    bool MovLIO(const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Response> response);
    bool MovJIO(const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Response> response);
    bool Arc(const std::shared_ptr<dobot_msgs_v4::srv::Arc::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Arc::Response> response);
    bool Circle(const std::shared_ptr<dobot_msgs_v4::srv::Circle::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Circle::Response> response);
    bool MoveJog(const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Response> response);
    bool StopMoveJog(const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Response> response);
    bool RelMovJTool(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Response> response);
    bool RelMovLTool(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Response> response);
    bool RelMovJUser(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Response> response);
    bool RelMovLUser(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Response> response);
    bool RelJointMovJ(const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Response> response);
    bool GetCurrentCommandId(const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Response> response);
    bool ServoJ(const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Response> response);
    bool ServoP(const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Response> response);

private:
    rclcpp::TimerBase::SharedPtr kTimer;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr kPublisherInfo;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::EnableRobot>> kServiceEnableRobot;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DisableRobot>> kServiceDisableRobot;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ClearError>> kServiceClearError;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SpeedFactor>> kServiceSpeedFactor;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::User>> kServiceUser;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::Tool>> kServiceTool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RobotMode>> kServiceRobotMode;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetPayload>> kServiceSetPayload;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DO>> kServiceDO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DOInstant>> kServiceDOInstant;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ToolDO>> kServiceToolDO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ToolDOInstant>> kServiceToolDOInstant;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::AO>> kServiceAO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::AOInstant>> kServiceAOInstant;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::AccJ>> kServiceAccJ;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::AccL>> kServiceAccL;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::VelJ>> kServiceVelJ;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::VelL>> kServiceVelL;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::CP>> kServiceCP;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::PowerOn>> kServicePowerOn;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RunScript>> kServiceRunScript;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::Stop>> kServiceStop;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::Pause>> kServicePause;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::EnableSafeSkin>> kServiceEnableSafeSkin;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetSafeSkin>> kServiceSetSafeSkin;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetStartPose>> kServiceGetStartPose;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::StartPath>> kServiceStartPath;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::PositiveKin>> kServicePositiveKin;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::InverseKin>> kServiceInverseKin;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetPose>> kServiceGetPose;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetAngle>> kServiceGetAngle;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetCollisionLevel>> kServiceSetCollisionLevel;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::EmergencyStop>> kServiceEmergencyStop;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ModbusRTUCreate>> kServiceModbusRTUCreate;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ModbusCreate>> kServiceModbusCreate;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ModbusClose>> kServiceModbusClose;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetInBits>> kServiceGetInBits;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetInRegs>> kServiceGetInRegs;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetCoils>> kServiceGetCoils;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetCoils>> kServiceSetCoils;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetHoldRegs>> kServiceGetHoldRegs;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetHoldRegs>> kServiceSetHoldRegs;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetErrorID>> kServiceGetErrorID;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DI>> kServiceDI;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ToolDI>> kServiceToolDI;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::AI>> kServiceAI;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ToolAI>> kServiceToolAI;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DIGroup>> kServiceDIGroup;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DOGroup>> kServiceDOGroup;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::BrakeControl>> kServiceBrakeControl;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::StartDrag>> kServiceStartDrag;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::StopDrag>> kServiceStopDrag;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::DragSensivity>> kServiceDragSensivity;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetDO>> kServiceGetDO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetAO>> kServiceGetAO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetDOGroup>> kServiceGetDOGroup;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetTool485>> kServiceSetTool485;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetSafeWallEnable>> kServiceSetSafeWallEnable;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetToolPower>> kServiceSetToolPower;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetToolMode>> kServiceSetToolMode;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetBackDistance>> kServiceSetBackDistance;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetPostCollisionMode>> kServiceSetPostCollisionMode;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetUser>> kServiceSetUser;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetTool>> kServiceSetTool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::CalcUser>> kServiceCalcUser;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::CalcTool>> kServiceCalcTool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetInputBool>> kServiceGetInputBool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetInputInt>> kServiceGetInputInt;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetInputFloat>> kServiceGetInputFloat;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetOutputBool>> kServiceGetOutputBool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetOutputInt>> kServiceGetOutputInt;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetOutputFloat>> kServiceGetOutputFloat;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetOutputBool>> kServiceSetOutputBool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetOutputInt>> kServiceSetOutputInt;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::SetOutputFloat>> kServiceSetOutputFloat;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::MovJ>> kServiceMovJ;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::MovL>> kServiceMovL;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::MovLIO>> kServiceMovLIO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::MovJIO>> kServiceMovJIO;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::Arc>> kServiceArc;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::Circle>> kServiceCircle;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::MoveJog>> kServiceMoveJog;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::StopMoveJog>> kServiceStopMoveJog;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RelMovJTool>> kServiceRelMovJTool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RelMovLTool>> kServiceRelMovLTool;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RelMovJUser>> kServiceRelMovJUser;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RelMovLUser>> kServiceRelMovLUser;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::RelJointMovJ>> kServiceRelJointMovJ;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::GetCurrentCommandId>> kServiceGetCurrentCommandId;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ServoJ>> kServiceServoJ;
    std::shared_ptr<rclcpp::Service<dobot_msgs_v4::srv::ServoP>> kServiceServoP;
    std::shared_ptr<rclcpp::Client<dobot_msgs_v4::srv::GetErrorID>> kClientGeterror;

private:
    void getErrorID(std::vector<int> &Vec);
    void pubFeedBackInfo();

private:
    std::string kRobotName;
    std::shared_ptr<CRCommanderRos2> commander_;
    std::thread threadPubFeedBackInfo;
};

#endif // CRROBOTROS2_H
