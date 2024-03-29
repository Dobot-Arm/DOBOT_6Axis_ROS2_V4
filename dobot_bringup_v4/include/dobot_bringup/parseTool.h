/**
 ***********************************************************************************************************************
 *
 * @author qiufengwu
 * @date   2024/01/23
 *
 *
 ***********************************************************************************************************************
 */
#ifndef PARSETOOL_HPP
#define PARSETOOL_HPP

#include <string>
#include <sstream>
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

namespace parseTool
{

    std::string parserenableRobotRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request);

    std::string parserdisableRobotRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Request> request);

    std::string parserclearErrorRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Request> request);

    std::string parserspeedFactorRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Request> request);

    std::string parseruserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::User::Request> request);

    std::string parsertoolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Tool::Request> request);

    std::string parserrobotModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Request> request);

    std::string parsersetPayloadRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Request> request);

    std::string parserDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DO::Request> request);

    std::string parserDOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Request> request);

    std::string parsertoolDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Request> request);

    std::string parsertoolDOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Request> request);

    std::string parserAORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AO::Request> request);

    std::string parserAOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Request> request);

    std::string parseraccJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Request> request);

    std::string parseraccLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AccL::Request> request);

    std::string parservelJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Request> request);

    std::string parservelLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::VelL::Request> request);

    std::string parsercpRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CP::Request> request);

    std::string parserpowerOnRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Request> request);

    std::string parserrunScriptRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Request> request);

    std::string parserstopRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Stop::Request> request);

    std::string parserpauseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Pause::Request> request);

    std::string parserContinueRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Continue::Request> request);

    std::string parserEnableSafeSkinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Request> request);

    std::string parserSetSafeSkinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Request> request);

    std::string parserGetStartPoseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Request> request);

    std::string parserStartPathRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Request> request);

    std::string parserPositiveKinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Request> request);

    std::string parserInverseKinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Request> request);

    std::string parserGetAngleRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Request> request);

    std::string parserGetPoseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Request> request);

    std::string parserEmergencyStopRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Request> request);

    std::string parsersetCollisionLevelRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Request> request);

    std::string parserModbusRTUCreateRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Request> request);

    std::string parserModbusCreateRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Request> request);

    std::string parserModbusCloseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Request> request);

    std::string parserGetInBitsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Request> request);

    std::string parserGetInRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Request> request);

    std::string parserGetCoilsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Request> request);

    std::string parserSetCoilsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Request> request);

    std::string parserGetHoldRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Request> request);

    std::string parserSetHoldRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Request> request);

    std::string parserGetErrorIDRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Request> request);

    std::string parserDIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DI::Request> request);

    std::string parserAIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AI::Request> request);

    std::string parserDIGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Request> request);

    std::string parserdoGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Request> request);

    std::string parserbrakeControlRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Request> request);

    std::string parserstartDragRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Request> request);

    std::string parserStopDragRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Request> request);

    std::string parserDragSensivityRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Request> request);

    std::string parserGetDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Request> request);

    std::string parserGetAORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Request> request);

    std::string parserGetDOGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Request> request);

    std::string parserSetTool485Request2String(const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Request> request);

    std::string parserSetSafeWallEnableRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Request> request);

    std::string parserSetToolPowerRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Request> request);

    std::string parserSetToolModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Request> request);

    std::string parserSetBackDistanceRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Request> request);

    std::string parserSetPostCollisionModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Request> request);

    std::string parserSetUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Request> request);

    std::string parserSetToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Request> request);

    std::string parserCalcUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Request> request);

    std::string parserCalcToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Request> request);

    std::string parserGetInputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Request> request);

    std::string parserGetInputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Request> request);

    std::string parserGetInputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Request> request);

    std::string parserGetOutputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Request> request);

    std::string parserGetOutputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Request> request);

    std::string parserGetOutputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Request> request);

    std::string parserSetOutputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Request> request);

    std::string parserSetOutputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Request> request);

    std::string parserSetOutputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Request> request);

    std::string parsermovJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Request> request);

    std::string parsermovLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovL::Request> request);

    std::string parserMovLIORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Request> request);

    std::string parserMovJIORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Request> request);

    std::string parserArcRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Arc::Request> request);

    std::string parsermoveJogRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Request> request);

    std::string parserStopMoveJogRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Request> request);

    std::string parserRelMovJToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Request> request);

    std::string parserRelMovLToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Request> request);

    std::string parserRelMovJUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Request> request);

    std::string parserRelMovLUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Request> request);

    std::string parserrelJointMovJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Request> request);

    std::string parserGetCurrentCommandIdRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Request> request);

    std::string parserServoJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Request> request);

    std::string parserServoPRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Request> request);

    std::string parserCircleRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Circle::Request> request);

    std::string parserToolAIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Request> request);

    std::string parserToolDIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Request> request);

}

#endif // PARSETOOL_HPP