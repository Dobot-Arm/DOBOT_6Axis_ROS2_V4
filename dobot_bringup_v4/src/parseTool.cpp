
#include <dobot_bringup/parseTool.h>

namespace parseTool
{

    std::string parserenableRobotRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "EnableRobot()";
        return ss.str();
    }
    std::string parserdisableRobotRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "DisableRobot()";
        return ss.str();
    }
    std::string parserclearErrorRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "ClearError()";
        return ss.str();
    }
    std::string parserspeedFactorRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Request> request)
    {
        std::stringstream ss;
        ss << "SpeedFactor(" << request->ratio << ")";
        return ss.str();
    }
    std::string parseruserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::User::Request> request)
    {
        std::stringstream ss;
        ss << "User(" << request->index << ")";
        return ss.str();
    }
    std::string parsertoolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Tool::Request> request)
    {
        std::stringstream ss;
        ss << "Tool(" << request->index << ")";
        return ss.str();
    }
    std::string parserrobotModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "RobotMode()";
        return ss.str();
    }
    std::string parsersetPayloadRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Request> request)
    {
        std::stringstream ss; // SetPayload(load,x,y,z)
        ss << "SetPayload(" << request->load << "," << request->x << "," << request->y << "," << request->z << ")";
        return ss.str();
    }
    std::string parserDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DO::Request> request)
    {
        std::stringstream ss; // DO(index,status,time)

        ss << "DO(" << request->index << "," << request->status << "," << request->time << ")";

        return ss.str();
    }
    std::string parserDOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Request> request)
    {
        std::stringstream ss;

        ss << "DOInstant(" << request->index << "," << request->status << ")";

        return ss.str();
    }
    std::string parsertoolDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Request> request)
    {
        std::stringstream ss;

        ss << "ToolDO(" << request->index << "," << request->status << ")";

        return ss.str();
    }
    std::string parsertoolDOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Request> request)
    {
        std::stringstream ss;

        ss << "ToolDOInstant(" << request->index << "," << request->status << ")";

        return ss.str();
    }
    std::string parserAORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AO::Request> request)
    {
        std::stringstream ss;
        ss << "AO(" << request->index << "," << request->value << ")";
        return ss.str();
    }
    std::string parserAOInstantRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Request> request)
    {
        std::stringstream ss; // AOInstant(index,value)

        ss << "AOInstant(" << request->index << "," << request->value << ")";

        return ss.str();
    }
    std::string parseraccJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Request> request)
    {
        std::stringstream ss; // AccJ(R)
        ss << "AccJ(" << request->r << ")";
        return ss.str();
    }
    std::string parseraccLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AccL::Request> request)
    {
        std::stringstream ss;
        ss << "AccL(" << request->r << ")";
        return ss.str();
    }
    std::string parservelJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Request> request)
    {
        std::stringstream ss;
        ss << "VelJ(" << request->r << ")";
        return ss.str();
    }
    std::string parservelLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::VelL::Request> request)
    {
        std::stringstream ss;
        ss << "VelL(" << request->r << ")";
        return ss.str();
    }
    std::string parsercpRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CP::Request> request)
    {
        std::stringstream ss;
        ss << "CP(" << request->r << ")";
        return ss.str();
    }
    std::string parserpowerOnRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;  // PowerOn()
        ss << "PowerOn()";
        return ss.str();
    }
    std::string parserrunScriptRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Request> request)
    {
        std::stringstream ss; // RunScript(projectName)
        ss << "RunScript(" << request->project_name << ")";
        return ss.str();
    }
    std::string parserstopRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Stop::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;  // Stop()
        ss << "Stop()";
        return ss.str();
    }
    std::string parserpauseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Pause::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;  // Pause()
        ss << "Pause()";
        return ss.str();
    }
    std::string parserContinueRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Continue::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;  // Continue()
        ss << "Continue()";
        return ss.str();
    }
    std::string parserEnableSafeSkinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Request> request)
    {
        std::stringstream ss;
        ss << "EnableSafeSkin(" << request->status << ")";
        return ss.str();
    }
    std::string parserSetSafeSkinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Request> request)
    {
        std::stringstream ss;
        ss << "SetSafeSkin(";
        ss << request->part << "," << request->status << ")";
        return ss.str();
    }
    std::string parserGetStartPoseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Request> request)
    {
        std::stringstream ss;
        ss << "GetStartPose(" << request->trace_name << ")";
        return ss.str();
    }
    std::string parserStartPathRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Request> request)
    {
        std::string stringOrder = "StartPath(";
        stringOrder = stringOrder + request->trace_name;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parserPositiveKinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Request> request)
    {
        std::stringstream ss;
        ss << "PositiveKin(";
        ss << request->j1 << "," << request->j2 << "," << request->j3 << "," << request->j4 << "," << request->j5 << ","
           << request->j6 << ",";
        if (request->user != "")
            ss << ",user=" << request->user;
        if (request->tool != "")
            ss << ", tool=" << request->tool;
        ss << ")";
        return ss.str();
    }
    std::string parserInverseKinRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Request> request)
    {
        // InverseKin(X,Y,Z,Rx,Ry,Rz,User,Tool,useJointNear,JointNear)
        std::stringstream ss;
        ss << "RunScript(" << request->x << "," << request->y << "," << request->z << "," << request->rx << "," << request->ry
           << "," << request->rz << ", user=" << request->user << ",tool=" << request->tool
           << ",useJointNear=" << request->use_joint_near << ",jointNear=" << request->joint_near << ")";
        return ss.str();
    }
    std::string parserGetAngleRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "GetAngle()";
        return ss.str();
    }
    std::string parserGetPoseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Request> request)
    {
        std::stringstream ss;
        ss << "GetPose("<<"user=" << request->user << ",tool=" << request->tool<<")";
        return ss.str();
    }
    std::string parserEmergencyStopRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Request> request)
    {
        std::stringstream ss;
        ss << "EmergencyStop(" << request->value << ")";
        return ss.str();
    }
    std::string parsersetCollisionLevelRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Request> request)
    {
        std::stringstream ss;
        ss << "SetCollisionLevel(" << request->level << ")";
        return ss.str();
    }
    std::string parserModbusRTUCreateRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Request> request)
    {
        std::stringstream ss;
        ss << "ModbusRTUCreate(" << request->slave_id << "," << request->baud << "," << request->parity << ","
           << request->data_bit << "," << request->stop_bit << ")";

        return ss.str();
    }
    std::string parserModbusCreateRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Request> request)
    {
        std::stringstream ss; // ModbusCreate(ip,port,slave_id,isRTU)
        ss << "ModbusCreate(" << request->ip << "," << request->port << "," << request->slave_id << "," << request->is_rtu
           << ")";

        return ss.str();
    }
    std::string parserModbusCloseRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Request> request)
    {
        std::stringstream ss;
        ss << "ModbusClose(" << request->index << ")";
        return ss.str();
    }
    std::string parserGetInBitsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Request> request)
    {
        std::stringstream ss;
        ss << "GetInBits(" << request->index << ", " << request->addr << ", " << request->count << ")";

        return ss.str();
    }
    std::string parserGetInRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Request> request)
    {
        std::stringstream ss;
        ss << "GetInRegs(request->" << request->index << ", request->" << request->addr << ", request->" << request->count
           << ", request->" << request->val_type << ")";

        return ss.str();
    }
    std::string parserGetCoilsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Request> request)
    {
        std::stringstream ss;
        ss << "GetCoils(request->" << request->index << ", request->" << request->addr << ", request->" << request->count << ")";

        return ss.str();
    }
    std::string parserSetCoilsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Request> request)
    {
        std::stringstream ss;
        ss << "SetCoils(request->" << request->index << ", request->" << request->addr << ", request->" << request->count
           << ", request->" << request->val_tab << ")";

        return ss.str();
    }
    std::string parserGetHoldRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Request> request)
    {
        std::stringstream ss;
        ss << "GetHoldRegs(request->" << request->index << ", request->" << request->addr << ", request->" << request->count
           << ", request->" << request->val_type << ")";

        return ss.str();
    }
    std::string parserSetHoldRegsRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Request> request)
    {
        std::stringstream ss;
        ss << "SetHoldRegs(" << request->index << "," << request->addr << "," << request->count
           << "," << request->val_tab << "," << request->val_type << ")";

        return ss.str();
    }
    std::string parserGetErrorIDRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "GetErrorID()";
        return ss.str();
    }
    std::string parserDIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DI::Request> request)
    {
        std::stringstream ss;
        ss << "DI(" << request->index << ")";
        return ss.str();
    }
    std::string parserAIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::AI::Request> request)
    {
        std::stringstream ss;
        ss << "AI(" << request->index << ")";
        return ss.str();
    }
    std::string parserDIGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Request> request)
    {
        std::stringstream ss;
        ss << "DIGroup(";

        for (size_t i = 0; i < request->args.size(); ++i)
        {
            if (i == 0)
            {
                ss << request->args[i];
            }
            else
                ss << "," << request->args[i];
        }
        ss << ")";

        return ss.str();
    }
    std::string parserdoGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Request> request)
    {
        std::stringstream ss;
        ss << "DOGroup(";
        for (size_t i = 0; i < request->args.size(); ++i)
        {
            if (i == 0)
            {
                ss << request->args[i];
            }
            else
                ss << "," << request->args[i];
        }
        ss << ")";

        return ss.str();
    }
    std::string parserbrakeControlRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Request> request)
    {
        std::stringstream ss;
        ss << "BrakeControl(" << request->axis_id << "," << request->value << ")";

        return ss.str();
    }
    std::string parserstartDragRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "StartDrag()";
        return ss.str();
    }
    std::string parserStopDragRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "StopDrag()";
        return ss.str();
    }
    std::string parserDragSensivityRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Request> request)
    {
        std::stringstream ss;
        ss << "DragSensivity(" << request->index << "," << request->value << ")";
        return ss.str();
    }
    std::string parserGetDORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Request> request)
    {
        std::stringstream ss;
        ss << "GetDO(" << request->index << ")";
        return ss.str();
    }
    std::string parserGetAORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Request> request)
    {
        std::stringstream ss;
        ss << " GetAO(" << request->index << ")";
        return ss.str();
    }
    std::string parserGetDOGroupRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Request> request)
    {
        std::stringstream ss;
        ss << "GetDOGroup(";
        for (size_t i = 0; i < request->index_group.size(); ++i)
        {
            if (i == 0)
                ss << request->index_group[i];
            else
                ss << "," << request->index_group[i];
        }
        ss << ")";
        return ss.str();
    }
    std::string parserSetTool485Request2String(const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Request> request)
    {
        std::stringstream ss;
        ss << "SetTool485(" << request->baudrate << ")";
        return ss.str();
    }
    std::string parserSetSafeWallEnableRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Request> request)
    {
        std::stringstream ss;
        ss << "SetSafeWallEnable(" << request->index << "," << request->value << ")";
        return ss.str();
    }
    std::string parserSetToolPowerRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Request> request)
    {
        std::stringstream ss;
        ss << "SetToolPower(" << request->status << ")";
        return ss.str();
    }
    std::string parserSetToolModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Request> request)
    {
        std::stringstream ss;
        ss << "SetToolMode(" << request->mode << "," << request->type << ")";
        return ss.str();
    }
    std::string parserSetBackDistanceRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Request> request)
    {
        std::stringstream ss;
        ss << "SetBackDistance(" << request->distance << ")";
        return ss.str();
    }
    std::string parserSetPostCollisionModeRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Request> request)
    {
        std::stringstream ss;
        ss << "SetPostCollisionMode(" << request->mode << ")";
        return ss.str();
    }
    std::string parserSetUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Request> request)
    {
        std::stringstream ss;
        ss << "SetUser(" << request->index << "," << request->value << ")";
        return ss.str();
    }
    std::string parserSetToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Request> request)
    {
        std::stringstream ss;
        ss << "SetTool(" << request->index << "," << request->value << ")";
        return ss.str();
    }
    std::string parserCalcUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Request> request)
    {
        std::stringstream ss;
        ss << "CalcUser(" << request->index << "," << request->matrix << "," << request->offset << ")";
        return ss.str();
    }
    std::string parserCalcToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Request> request)
    {
        std::stringstream ss;
        ss << "CalcTool(" << request->index << "," << request->matrix << "," << request->offset << ")";
        return ss.str();
    }
    std::string parserGetInputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Request> request)
    {
        std::stringstream ss;
        ss << "GetInputBool(" << request->address << ")";
        return ss.str();
    }
    std::string parserGetInputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Request> request)
    {
        std::stringstream ss;
        ss << "GetInputInt(" << request->address << ")";
        return ss.str();
    }
    std::string parserGetInputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Request> request)
    {
        std::stringstream ss;
        ss << "GetInputFloat(" << request->address << ")";
        return ss.str();
    }
    std::string parserGetOutputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Request> request)
    {
        std::stringstream ss;
        ss << "GetOutputBool(" << request->address << ")";
        return ss.str();
    }
    std::string parserGetOutputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Request> request)
    {
        std::stringstream ss;
        ss << "GetOutputInt(" << request->address << ")";
        return ss.str();
    }
    std::string parserGetOutputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Request> request)
    {
        std::stringstream ss;
        ss << "GetOutputFloat(" << request->address << ")";
        return ss.str();
    }
    std::string parserSetOutputboolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Request> request)
    {
        std::stringstream ss;
        ss << "SetOutputBool(" << request->address << "," << request->value << ")";
        return ss.str();
    }
    std::string parserSetOutputIntRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Request> request)
    {
        std::stringstream ss;
        ss << "SetOutputInt(" << request->address << "," << request->value << ")";

        return ss.str();
    }
    std::string parserSetOutputFloatRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Request> request)
    {
        std::stringstream ss;
        ss << "SetOutputFloat(" << request->address << "," << request->value << ")";
        return ss.str();
    }
    std::string parsermovJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Request> request)
    {
        std::string stringOrder = "MovJ(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);

        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate;
        }
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parsermovLRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovL::Request> request)
    {
        std::string stringOrder = "MovL(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);

        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate;
        }
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parserMovLIORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Request> request)
    {
        std::string stringOrder = "MovLIO(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);

        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate;
        }

        if (!(request->mdis.empty() || request->mdis[0].empty()))
        {
            for (size_t i = 0; i < request->mdis.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->mdis[i]);
            }
        }

        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserMovJIORequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Request> request)
    {
        std::string stringOrder = "MovJIO(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);

        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate;
        }

        if (!(request->mdis.empty() || request->mdis[0].empty()))
        {
            for (size_t i = 0; i < request->mdis.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->mdis[i]);
            }
        }

        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserArcRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Arc::Request> request)
    {
        std::string stringOrder = "Arc(";
        char cmdCoordinate[100];
        char cmdCoordinate2[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);
        sprintf(cmdCoordinate2, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a2, request->b2, request->c2, request->d2,
                request->e2, request->f2);
        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate + "," + "joint=" + cmdCoordinate2;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate + "," + "pose=" + cmdCoordinate2;
        }

        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parsermoveJogRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Request> request)
    {
        std::string stringOrder = "MoveJog(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%s", request->axis_id.c_str());
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserStopMoveJogRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::string stringOrder = "MoveJog()";

        return stringOrder;
    }

    std::string parserRelMovJToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Request> request)
    {
        std::string stringOrder = "RelMovJTool(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parserRelMovLToolRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Request> request)
    {
        std::string stringOrder = "RelMovLTool(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parserRelMovJUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Request> request)
    {
        std::string stringOrder = "RelMovJUser(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserRelMovLUserRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Request> request)
    {
        std::string stringOrder = "RelMovLUser(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserrelJointMovJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Request> request)
    {
        std::string stringOrder = "RelJointMovJ(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }
    std::string parserGetCurrentCommandIdRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Request> request)
    {
        std::ignore = request; // 明确地忽略参数
        std::stringstream ss;
        ss << "GetCurrentCommandID()";
        return ss.str();
    }
    std::string parserServoJRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Request> request)
    {
        std::string stringOrder = "ServoJ(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserServoPRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Request> request)
    {
        std::string stringOrder = "ServoP(";
        char cmdCoordinate[100];
        sprintf(cmdCoordinate, "%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f", request->a, request->b, request->c, request->d, request->e,
                request->f);
        stringOrder = stringOrder + cmdCoordinate;
        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserCircleRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::Circle::Request> request)
    {
        std::stringstream ss;
        std::string stringOrder = "Circle(";
        char cmdCoordinate[100];
        char cmdCoordinate2[100];
        sprintf(cmdCoordinate, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a, request->b, request->c, request->d,
                request->e, request->f);
        sprintf(cmdCoordinate2, "{%0.3f,%0.3f,%0.3f,%0.3f,%0.3f,%0.3f}", request->a2, request->b2, request->c2, request->d2,
                request->e2, request->f2);
        if (request->mode)
        {
            stringOrder = stringOrder + "joint=" + cmdCoordinate + "," + "joint=" + cmdCoordinate2;
        }
        else
        {
            stringOrder = stringOrder + "pose=" + cmdCoordinate + "," + "pose=" + cmdCoordinate2;
        }

        stringOrder = stringOrder + "," + std::to_string(request->count);

        if (!(request->param_value.empty() || request->param_value[0].empty()))
        {
            for (size_t i = 0; i < request->param_value.size(); i++)
            {
                stringOrder = stringOrder + "," + std::string(request->param_value[i]);
            }
        }

        stringOrder = stringOrder + ")";

        return stringOrder;
    }

    std::string parserToolAIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Request> request)
    {
        std::stringstream ss;
        ss << "ToolAI(" << request->index << ")";
        return ss.str();
    }
    std::string parserToolDIRequest2String(const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Request> request)
    {
        std::stringstream ss;
        ss << "ToolDI(" << request->index << ")";
        return ss.str();
    }
}
