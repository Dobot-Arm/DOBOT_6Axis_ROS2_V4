
#include <dobot_bringup/cr_robot_ros2.h>
#include <sensor_msgs/msg/joint_state.hpp>

CRRobotRos2::CRRobotRos2() : rclcpp::Node("dobot_bringup_ros2"){};

void CRRobotRos2::init()
{
    std::string robotIp{""};
    std::string robotType{""};
    double trajectoryDuration{0.0};
    std::string robotNodeName{""};
    int robotNumber = 1;

    // 获取参数 dobot_bringup_ros2.launch.py
    this->declare_parameter("robot_ip_address", "192.168.1.6");
    this->declare_parameter("robot_type", "cr5");
    this->declare_parameter("trajectory_duration", 0.3);
    this->declare_parameter("robot_node_name", "dobot_bringup_ros2");
    this->declare_parameter("robot_number", 1);

    this->get_parameter("robot_ip_address", robotIp);
    this->get_parameter("robot_type", robotType);
    this->get_parameter("trajectory_duration", trajectoryDuration);
    this->get_parameter("robot_node_name", robotNodeName);
    this->get_parameter("robot_number", robotNumber);

    RCLCPP_INFO(this->get_logger(), "robotIp %s", robotIp.c_str());
    RCLCPP_INFO(this->get_logger(), "robotType  %s", robotType.c_str());
    RCLCPP_INFO(this->get_logger(), "trajectoryDuration %f", trajectoryDuration);

    if (robotNumber > 1)
    {
        kRobotName = robotNodeName + "/";
    }

    std::string serviceEnableRobot = kRobotName + "/dobot_bringup_ros2/srv/EnableRobot";
    std::string serviceDisableRobot = kRobotName + "/dobot_bringup_ros2/srv/DisableRobot";
    std::string serviceClearError = kRobotName + "/dobot_bringup_ros2/srv/ClearError";
    std::string serviceSpeedFactor = kRobotName + "/dobot_bringup_ros2/srv/SpeedFactor";
    std::string serviceUser = kRobotName + "/dobot_bringup_ros2/srv/User";
    std::string serviceTool = kRobotName + "/dobot_bringup_ros2/srv/Tool";
    std::string serviceRobotMode = kRobotName + "/dobot_bringup_ros2/srv/RobotMode";
    std::string serviceSetPayload = kRobotName + "/dobot_bringup_ros2/srv/SetPayload";
    std::string serviceDO = kRobotName + "/dobot_bringup_ros2/srv/DO";
    std::string serviceDOInstant = kRobotName + "/dobot_bringup_ros2/srv/DOInstant";
    std::string serviceToolDO = kRobotName + "/dobot_bringup_ros2/srv/ToolDO";
    std::string serviceToolDOInstant = kRobotName + "/dobot_bringup_ros2/srv/ToolDOInstant";
    std::string serviceAO = kRobotName + "/dobot_bringup_ros2/srv/AO";
    std::string serviceAOInstant = kRobotName + "/dobot_bringup_ros2/srv/AOInstant";
    std::string serviceAccJ = +"/dobot_bringup_ros2/srv/AccJ";
    std::string serviceAccL = kRobotName + "/dobot_bringup_ros2/srv/AccL";
    std::string serviceVelJ = kRobotName + "/dobot_bringup_ros2/srv/VelJ";
    std::string serviceVelL = kRobotName + "/dobot_bringup_ros2/srv/VelL";
    std::string serviceCP = kRobotName + "/dobot_bringup_ros2/srv/CP";
    std::string servicePowerOn = kRobotName + "/dobot_bringup_ros2/srv/PowerOn";
    std::string serviceRunScript = kRobotName + "/dobot_bringup_ros2/srv/RunScript";
    std::string serviceStop = kRobotName + "/dobot_bringup_ros2/srv/Stop";
    std::string servicePause = kRobotName + "/dobot_bringup_ros2/srv/Pause";
    std::string serviceContinue = kRobotName + "/dobot_bringup_ros2/srv/Continue";

    std::string serviceEnableSafeSkin = kRobotName + "/dobot_bringup_ros2/srv/EnableSafeSkin";
    std::string serviceSetSafeSkin = kRobotName + "/dobot_bringup_ros2/srv/SetSafeSkin";
    std::string serviceGetStartPose = kRobotName + "/dobot_bringup_ros2/srv/GetStartPose";
    std::string serviceStartPath = kRobotName + "/dobot_bringup_ros2/srv/StartPatht";
    std::string servicePositiveKin = kRobotName + "/dobot_bringup_ros2/srv/PositiveKin";
    std::string serviceInverseKin = kRobotName + "/dobot_bringup_ros2/srv/InverseKin";
    std::string serviceGetAngle = kRobotName + "/dobot_bringup_ros2/srv/GetAngle";
    std::string serviceGetPose = kRobotName + "/dobot_bringup_ros2/srv/GetPose";
    std::string serviceSetCollisionLevel = kRobotName + "/dobot_bringup_ros2/srv/SetCollisionLevel";
    std::string serviceEmergencyStop = kRobotName + "/dobot_bringup_ros2/srv/EmergencyStop";
    std::string serviceModbusRTUCreate = kRobotName + "/dobot_bringup_ros2/srv/ModbusRTUCreate";
    std::string serviceModbusCreate = kRobotName + "/dobot_bringup_ros2/srv/ModbusCreate";
    std::string serviceModbusClose = kRobotName + "/dobot_bringup_ros2/srv/ModbusClose";
    std::string serviceGetInBits = kRobotName + "/dobot_bringup_ros2/srv/GetInBits";
    std::string serviceGetInRegs = kRobotName + "/dobot_bringup_ros2/srv/GetInRegs";
    std::string serviceGetCoils = kRobotName + "/dobot_bringup_ros2/srv/GetCoils";
    std::string serviceSetCoils = kRobotName + "/dobot_bringup_ros2/srv/SetCoils";
    std::string serviceGetHoldRegs = kRobotName + "/dobot_bringup_ros2/srv/GetHoldRegs";
    std::string serviceSetHoldRegs = kRobotName + "/dobot_bringup_ros2/srv/SetHoldRegs";

    std::string serviceGetErrorID = kRobotName + "/dobot_bringup_ros2/srv/GetErrorID";
    std::string serviceDI = kRobotName + "/dobot_bringup_ros2/srv/DI";
    std::string serviceToolDI = kRobotName + "/dobot_bringup_ros2/srv/ToolDI";
    std::string serviceAI = kRobotName + "/dobot_bringup_ros2/srv/AI";
    std::string serviceToolAI = kRobotName + "/dobot_bringup_ros2/srv/ToolAI";
    std::string serviceDIGroup = kRobotName + "/dobot_bringup_ros2/srv/DIGroup";
    std::string serviceDOGroup = kRobotName + "/dobot_bringup_ros2/srv/DoGroup";
    std::string serviceBrakeControl = kRobotName + "/dobot_bringup_ros2/srv/BrakeControl";
    std::string serviceStartDrag = kRobotName + "/dobot_bringup_ros2/srv/StartDrag";
    std::string serviceStopDrag = kRobotName + "/dobot_bringup_ros2/srv/StopDrag";
    std::string serviceDragSensivity = kRobotName + "/dobot_bringup_ros2/srv/DragSensivity";
    std::string serviceGetDO = kRobotName + "/dobot_bringup_ros2/srv/GetDO";
    std::string serviceGetAO = kRobotName + "/dobot_bringup_ros2/srv/GetAO";
    std::string serviceGetDOGroup = kRobotName + "/dobot_bringup_ros2/srv/GetDOGroup";
    std::string serviceSetTool485 = kRobotName + "/dobot_bringup_ros2/srv/SetTool485";
    std::string serviceSetSafeWallEnable = kRobotName + "/dobot_bringup_ros2/srv/SetSafeWallEnable";
    std::string serviceSetToolPower = kRobotName + "/dobot_bringup_ros2/srv/SetToolPower";
    std::string serviceSetToolMode = kRobotName + "/dobot_bringup_ros2/srv/SetToolMode";
    std::string serviceSetBackDistance = kRobotName + "/dobot_bringup_ros2/srv/SetBackDistance";
    std::string serviceSetPostCollisionMode = kRobotName + "/dobot_bringup_ros2/srv/SetPostCollisionMode";
    std::string serviceSetUser = kRobotName + "/dobot_bringup_ros2/srv/SetUser";
    std::string serviceSetTool = kRobotName + "/dobot_bringup_ros2/srv/SetTool";
    std::string serviceCalcUser = kRobotName + "/dobot_bringup_ros2/srv/CalcUser";
    std::string serviceCalcTool = kRobotName + "/dobot_bringup_ros2/srv/CalcTool";
    std::string serviceGetInputBool = kRobotName + "/dobot_bringup_ros2/srv/GetInputBool";
    std::string serviceGetInputInt = kRobotName + "/dobot_bringup_ros2/srv/GetInputInt";
    std::string serviceGetInputFloat = kRobotName + "/dobot_bringup_ros2/srv/GetInputFloat";
    std::string serviceGetOutputBool = kRobotName + "/dobot_bringup_ros2/srv/GetOutputBool";
    std::string serviceGetOutputInt = kRobotName + "/dobot_bringup_ros2/srv/GetOutputInt";
    std::string serviceGetOutputFloat = kRobotName + "/dobot_bringup_ros2/srv/GetOutputFloat";
    std::string serviceSetOutputBool = kRobotName + "/dobot_bringup_ros2/srv/SetOutputBool";
    std::string serviceSetOutputInt = kRobotName + "/dobot_bringup_ros2/srv/SetOutputInt";
    std::string serviceSetOutputFloat = kRobotName + "/dobot_bringup_ros2/srv/SetOutputFloat";

    std::string serviceMovJ = kRobotName + "/dobot_bringup_ros2/srv/MovJ";
    std::string serviceMovL = kRobotName + "/dobot_bringup_ros2/srv/MovL";
    std::string serviceMovLIO = kRobotName + "/dobot_bringup_ros2/srv/MovLIO";
    std::string serviceMovJIO = kRobotName + "/dobot_bringup_ros2/srv/MovJIO";
    std::string serviceArc = kRobotName + "/dobot_bringup_ros2/srv/Arc";
    std::string serviceCircle = kRobotName + "/dobot_bringup_ros2/srv/Circle";
    std::string serviceMoveJog = kRobotName + "/dobot_bringup_ros2/srv/MoveJog";
    std::string serviceStopMoveJog = kRobotName + "/dobot_bringup_ros2/srv/StopMoveJog";
    std::string serviceRelMovJTool = kRobotName + "/dobot_bringup_ros2/srv/RelMovJTool";
    std::string serviceRelMovLTool = kRobotName + "/dobot_bringup_ros2/srv/RelMovLTool";
    std::string serviceRelMovJUser = kRobotName + "/dobot_bringup_ros2/srv/RelMovJUser";
    std::string serviceRelMovLUser = kRobotName + "/dobot_bringup_ros2/srv/RelMovLUser";
    std::string serviceRelJointMovJ = kRobotName + "/dobot_bringup_ros2/srv/RelJointMovJ";
    std::string serviceGetCurrentCommandId = kRobotName + "/dobot_bringup_ros2/srv/GetCurrentCommandId";
    std::string serviceServoJ = kRobotName + "/dobot_bringup_ros2/srv/ServoJ";
    std::string serviceServoP = kRobotName + "/dobot_bringup_ros2/srv/ServoP";
    std::string topicFeedInfo = kRobotName + "/dobot_bringup_ros2/msg/FeedInfo";

    kServiceEnableRobot = this->create_service<dobot_msgs_v4::srv::EnableRobot>(serviceEnableRobot, std::bind(&CRRobotRos2::EnableRobot, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDisableRobot = this->create_service<dobot_msgs_v4::srv::DisableRobot>(serviceDisableRobot, std::bind(&CRRobotRos2::DisableRobot, this, std::placeholders::_1, std::placeholders::_2));
    kServiceClearError = this->create_service<dobot_msgs_v4::srv::ClearError>(serviceClearError, std::bind(&CRRobotRos2::ClearError, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSpeedFactor = this->create_service<dobot_msgs_v4::srv::SpeedFactor>(serviceSpeedFactor, std::bind(&CRRobotRos2::SpeedFactor, this, std::placeholders::_1, std::placeholders::_2));
    kServiceUser = this->create_service<dobot_msgs_v4::srv::User>(serviceUser, std::bind(&CRRobotRos2::User, this, std::placeholders::_1, std::placeholders::_2));
    kServiceTool = this->create_service<dobot_msgs_v4::srv::Tool>(serviceTool, std::bind(&CRRobotRos2::Tool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRobotMode = this->create_service<dobot_msgs_v4::srv::RobotMode>(serviceRobotMode, std::bind(&CRRobotRos2::RobotMode, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetPayload = this->create_service<dobot_msgs_v4::srv::SetPayload>(serviceSetPayload, std::bind(&CRRobotRos2::SetPayload, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDO = this->create_service<dobot_msgs_v4::srv::DO>(serviceDO, std::bind(&CRRobotRos2::DO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDOInstant = this->create_service<dobot_msgs_v4::srv::DOInstant>(serviceDOInstant, std::bind(&CRRobotRos2::DOInstant, this, std::placeholders::_1, std::placeholders::_2));
    kServiceToolDO = this->create_service<dobot_msgs_v4::srv::ToolDO>(serviceToolDO, std::bind(&CRRobotRos2::ToolDO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceToolDOInstant = this->create_service<dobot_msgs_v4::srv::ToolDOInstant>(serviceToolDOInstant, std::bind(&CRRobotRos2::ToolDOInstant, this, std::placeholders::_1, std::placeholders::_2));
    kServiceAO = this->create_service<dobot_msgs_v4::srv::AO>(serviceAO, std::bind(&CRRobotRos2::AO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceAOInstant = this->create_service<dobot_msgs_v4::srv::AOInstant>(serviceAOInstant, std::bind(&CRRobotRos2::AOInstant, this, std::placeholders::_1, std::placeholders::_2));
    kServiceAccJ = this->create_service<dobot_msgs_v4::srv::AccJ>(serviceAccJ, std::bind(&CRRobotRos2::AccJ, this, std::placeholders::_1, std::placeholders::_2));
    kServiceAccL = this->create_service<dobot_msgs_v4::srv::AccL>(serviceAccL, std::bind(&CRRobotRos2::AccL, this, std::placeholders::_1, std::placeholders::_2));
    kServiceVelJ = this->create_service<dobot_msgs_v4::srv::VelJ>(serviceVelJ, std::bind(&CRRobotRos2::VelJ, this, std::placeholders::_1, std::placeholders::_2));
    kServiceVelL = this->create_service<dobot_msgs_v4::srv::VelL>(serviceVelL, std::bind(&CRRobotRos2::VelL, this, std::placeholders::_1, std::placeholders::_2));
    kServiceCP = this->create_service<dobot_msgs_v4::srv::CP>(serviceCP, std::bind(&CRRobotRos2::CP, this, std::placeholders::_1, std::placeholders::_2));
    kServicePowerOn = this->create_service<dobot_msgs_v4::srv::PowerOn>(servicePowerOn, std::bind(&CRRobotRos2::PowerOn, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRunScript = this->create_service<dobot_msgs_v4::srv::RunScript>(serviceRunScript, std::bind(&CRRobotRos2::RunScript, this, std::placeholders::_1, std::placeholders::_2));
    kServiceStop = this->create_service<dobot_msgs_v4::srv::Stop>(serviceStop, std::bind(&CRRobotRos2::Stop, this, std::placeholders::_1, std::placeholders::_2));
    kServicePause = this->create_service<dobot_msgs_v4::srv::Pause>(servicePause, std::bind(&CRRobotRos2::Pause, this, std::placeholders::_1, std::placeholders::_2));
    kServiceEnableSafeSkin = this->create_service<dobot_msgs_v4::srv::EnableSafeSkin>(serviceEnableSafeSkin, std::bind(&CRRobotRos2::EnableSafeSkin, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetSafeSkin = this->create_service<dobot_msgs_v4::srv::SetSafeSkin>(serviceSetSafeSkin, std::bind(&CRRobotRos2::SetSafeSkin, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetStartPose = this->create_service<dobot_msgs_v4::srv::GetStartPose>(serviceGetStartPose, std::bind(&CRRobotRos2::GetStartPose, this, std::placeholders::_1, std::placeholders::_2));
    kServiceStartPath = this->create_service<dobot_msgs_v4::srv::StartPath>(serviceStartPath, std::bind(&CRRobotRos2::StartPath, this, std::placeholders::_1, std::placeholders::_2));
    kServicePositiveKin = this->create_service<dobot_msgs_v4::srv::PositiveKin>(servicePositiveKin, std::bind(&CRRobotRos2::PositiveKin, this, std::placeholders::_1, std::placeholders::_2));
    kServiceInverseKin = this->create_service<dobot_msgs_v4::srv::InverseKin>(serviceInverseKin, std::bind(&CRRobotRos2::InverseKin, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetPose = this->create_service<dobot_msgs_v4::srv::GetPose>(serviceGetPose, std::bind(&CRRobotRos2::GetPose, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetAngle = this->create_service<dobot_msgs_v4::srv::GetAngle>(serviceGetAngle, std::bind(&CRRobotRos2::GetAngle, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetCollisionLevel = this->create_service<dobot_msgs_v4::srv::SetCollisionLevel>(serviceSetCollisionLevel, std::bind(&CRRobotRos2::SetCollisionLevel, this, std::placeholders::_1, std::placeholders::_2));
    kServiceEmergencyStop = this->create_service<dobot_msgs_v4::srv::EmergencyStop>(serviceEmergencyStop, std::bind(&CRRobotRos2::EmergencyStop, this, std::placeholders::_1, std::placeholders::_2));
    kServiceModbusRTUCreate = this->create_service<dobot_msgs_v4::srv::ModbusRTUCreate>(serviceModbusRTUCreate, std::bind(&CRRobotRos2::ModbusRTUCreate, this, std::placeholders::_1, std::placeholders::_2));
    kServiceModbusCreate = this->create_service<dobot_msgs_v4::srv::ModbusCreate>(serviceModbusCreate, std::bind(&CRRobotRos2::ModbusCreate, this, std::placeholders::_1, std::placeholders::_2));
    kServiceModbusClose = this->create_service<dobot_msgs_v4::srv::ModbusClose>(serviceModbusClose, std::bind(&CRRobotRos2::ModbusClose, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetInBits = this->create_service<dobot_msgs_v4::srv::GetInBits>(serviceGetInBits, std::bind(&CRRobotRos2::GetInBits, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetInRegs = this->create_service<dobot_msgs_v4::srv::GetInRegs>(serviceGetInRegs, std::bind(&CRRobotRos2::GetInRegs, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetCoils = this->create_service<dobot_msgs_v4::srv::GetCoils>(serviceGetCoils, std::bind(&CRRobotRos2::GetCoils, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetCoils = this->create_service<dobot_msgs_v4::srv::SetCoils>(serviceSetCoils, std::bind(&CRRobotRos2::SetCoils, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetHoldRegs = this->create_service<dobot_msgs_v4::srv::GetHoldRegs>(serviceGetHoldRegs, std::bind(&CRRobotRos2::GetHoldRegs, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetHoldRegs = this->create_service<dobot_msgs_v4::srv::SetHoldRegs>(serviceSetHoldRegs, std::bind(&CRRobotRos2::SetHoldRegs, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetErrorID = this->create_service<dobot_msgs_v4::srv::GetErrorID>(serviceGetErrorID, std::bind(&CRRobotRos2::GetErrorID, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDI = this->create_service<dobot_msgs_v4::srv::DI>(serviceDI, std::bind(&CRRobotRos2::DI, this, std::placeholders::_1, std::placeholders::_2));
    kServiceToolDI = this->create_service<dobot_msgs_v4::srv::ToolDI>(serviceToolDI, std::bind(&CRRobotRos2::ToolDI, this, std::placeholders::_1, std::placeholders::_2));
    kServiceAI = this->create_service<dobot_msgs_v4::srv::AI>(serviceAI, std::bind(&CRRobotRos2::AI, this, std::placeholders::_1, std::placeholders::_2));
    kServiceToolAI = this->create_service<dobot_msgs_v4::srv::ToolAI>(serviceToolAI, std::bind(&CRRobotRos2::ToolAI, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDIGroup = this->create_service<dobot_msgs_v4::srv::DIGroup>(serviceDIGroup, std::bind(&CRRobotRos2::DIGroup, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDOGroup = this->create_service<dobot_msgs_v4::srv::DOGroup>(serviceDOGroup, std::bind(&CRRobotRos2::DOGroup, this, std::placeholders::_1, std::placeholders::_2));
    kServiceBrakeControl = this->create_service<dobot_msgs_v4::srv::BrakeControl>(serviceBrakeControl, std::bind(&CRRobotRos2::BrakeControl, this, std::placeholders::_1, std::placeholders::_2));
    kServiceStartDrag = this->create_service<dobot_msgs_v4::srv::StartDrag>(serviceStartDrag, std::bind(&CRRobotRos2::StartDrag, this, std::placeholders::_1, std::placeholders::_2));
    kServiceStopDrag = this->create_service<dobot_msgs_v4::srv::StopDrag>(serviceStopDrag, std::bind(&CRRobotRos2::StopDrag, this, std::placeholders::_1, std::placeholders::_2));
    kServiceDragSensivity = this->create_service<dobot_msgs_v4::srv::DragSensivity>(serviceDragSensivity, std::bind(&CRRobotRos2::DragSensivity, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetDO = this->create_service<dobot_msgs_v4::srv::GetDO>(serviceGetDO, std::bind(&CRRobotRos2::GetDO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetAO = this->create_service<dobot_msgs_v4::srv::GetAO>(serviceGetAO, std::bind(&CRRobotRos2::GetAO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetDOGroup = this->create_service<dobot_msgs_v4::srv::GetDOGroup>(serviceGetDOGroup, std::bind(&CRRobotRos2::GetDOGroup, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetTool485 = this->create_service<dobot_msgs_v4::srv::SetTool485>(serviceSetTool485, std::bind(&CRRobotRos2::SetTool485, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetSafeWallEnable = this->create_service<dobot_msgs_v4::srv::SetSafeWallEnable>(serviceSetSafeWallEnable, std::bind(&CRRobotRos2::SetSafeWallEnable, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetToolPower = this->create_service<dobot_msgs_v4::srv::SetToolPower>(serviceSetToolPower, std::bind(&CRRobotRos2::SetToolPower, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetToolMode = this->create_service<dobot_msgs_v4::srv::SetToolMode>(serviceSetToolMode, std::bind(&CRRobotRos2::SetToolMode, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetBackDistance = this->create_service<dobot_msgs_v4::srv::SetBackDistance>(serviceSetBackDistance, std::bind(&CRRobotRos2::SetBackDistance, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetPostCollisionMode = this->create_service<dobot_msgs_v4::srv::SetPostCollisionMode>(serviceSetPostCollisionMode, std::bind(&CRRobotRos2::SetPostCollisionMode, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetUser = this->create_service<dobot_msgs_v4::srv::SetUser>(serviceSetUser, std::bind(&CRRobotRos2::SetUser, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetTool = this->create_service<dobot_msgs_v4::srv::SetTool>(serviceSetTool, std::bind(&CRRobotRos2::SetTool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceCalcUser = this->create_service<dobot_msgs_v4::srv::CalcUser>(serviceCalcUser, std::bind(&CRRobotRos2::CalcUser, this, std::placeholders::_1, std::placeholders::_2));
    kServiceCalcTool = this->create_service<dobot_msgs_v4::srv::CalcTool>(serviceCalcTool, std::bind(&CRRobotRos2::CalcTool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetInputBool = this->create_service<dobot_msgs_v4::srv::GetInputBool>(serviceGetInputBool, std::bind(&CRRobotRos2::GetInputBool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetInputInt = this->create_service<dobot_msgs_v4::srv::GetInputInt>(serviceGetInputInt, std::bind(&CRRobotRos2::GetInputInt, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetInputFloat = this->create_service<dobot_msgs_v4::srv::GetInputFloat>(serviceGetInputFloat, std::bind(&CRRobotRos2::GetInputFloat, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetOutputBool = this->create_service<dobot_msgs_v4::srv::GetOutputBool>(serviceGetOutputBool, std::bind(&CRRobotRos2::GetOutputBool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetOutputInt = this->create_service<dobot_msgs_v4::srv::GetOutputInt>(serviceGetOutputInt, std::bind(&CRRobotRos2::GetOutputInt, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetOutputFloat = this->create_service<dobot_msgs_v4::srv::GetOutputFloat>(serviceGetOutputFloat, std::bind(&CRRobotRos2::GetOutputFloat, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetOutputBool = this->create_service<dobot_msgs_v4::srv::SetOutputBool>(serviceSetOutputBool, std::bind(&CRRobotRos2::SetOutputBool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetOutputInt = this->create_service<dobot_msgs_v4::srv::SetOutputInt>(serviceSetOutputInt, std::bind(&CRRobotRos2::SetOutputInt, this, std::placeholders::_1, std::placeholders::_2));
    kServiceSetOutputFloat = this->create_service<dobot_msgs_v4::srv::SetOutputFloat>(serviceSetOutputFloat, std::bind(&CRRobotRos2::SetOutputFloat, this, std::placeholders::_1, std::placeholders::_2));
    kServiceMovJ = this->create_service<dobot_msgs_v4::srv::MovJ>(serviceMovJ, std::bind(&CRRobotRos2::MovJ, this, std::placeholders::_1, std::placeholders::_2));
    kServiceMovL = this->create_service<dobot_msgs_v4::srv::MovL>(serviceMovL, std::bind(&CRRobotRos2::MovL, this, std::placeholders::_1, std::placeholders::_2));
    kServiceMovLIO = this->create_service<dobot_msgs_v4::srv::MovLIO>(serviceMovLIO, std::bind(&CRRobotRos2::MovLIO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceMovJIO = this->create_service<dobot_msgs_v4::srv::MovJIO>(serviceMovJIO, std::bind(&CRRobotRos2::MovJIO, this, std::placeholders::_1, std::placeholders::_2));
    kServiceArc = this->create_service<dobot_msgs_v4::srv::Arc>(serviceArc, std::bind(&CRRobotRos2::Arc, this, std::placeholders::_1, std::placeholders::_2));
    kServiceCircle = this->create_service<dobot_msgs_v4::srv::Circle>(serviceCircle, std::bind(&CRRobotRos2::Circle, this, std::placeholders::_1, std::placeholders::_2));
    kServiceMoveJog = this->create_service<dobot_msgs_v4::srv::MoveJog>(serviceMoveJog, std::bind(&CRRobotRos2::MoveJog, this, std::placeholders::_1, std::placeholders::_2));
    kServiceStopMoveJog = this->create_service<dobot_msgs_v4::srv::StopMoveJog>(serviceStopMoveJog, std::bind(&CRRobotRos2::StopMoveJog, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRelMovLTool = this->create_service<dobot_msgs_v4::srv::RelMovLTool>(serviceRelMovLTool, std::bind(&CRRobotRos2::RelMovLTool, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRelMovJUser = this->create_service<dobot_msgs_v4::srv::RelMovJUser>(serviceRelMovJUser, std::bind(&CRRobotRos2::RelMovJUser, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRelMovLUser = this->create_service<dobot_msgs_v4::srv::RelMovLUser>(serviceRelMovLUser, std::bind(&CRRobotRos2::RelMovLUser, this, std::placeholders::_1, std::placeholders::_2));
    kServiceRelJointMovJ = this->create_service<dobot_msgs_v4::srv::RelJointMovJ>(serviceRelJointMovJ, std::bind(&CRRobotRos2::RelJointMovJ, this, std::placeholders::_1, std::placeholders::_2));
    kServiceGetCurrentCommandId = this->create_service<dobot_msgs_v4::srv::GetCurrentCommandId>(serviceGetCurrentCommandId, std::bind(&CRRobotRos2::GetCurrentCommandId, this, std::placeholders::_1, std::placeholders::_2));
    kServiceServoJ = this->create_service<dobot_msgs_v4::srv::ServoJ>(serviceServoJ, std::bind(&CRRobotRos2::ServoJ, this, std::placeholders::_1, std::placeholders::_2));
    kServiceServoP = this->create_service<dobot_msgs_v4::srv::ServoP>(serviceServoP, std::bind(&CRRobotRos2::ServoP, this, std::placeholders::_1, std::placeholders::_2));

    //kTimer = this->create_wall_timer(std::chrono::seconds(2), std::bind(&CRRobotRos2::backendTask, this));
    commander_ = std::make_shared<CRCommanderRos2>(robotIp);
    commander_->init();
    kPublisherInfo = this->create_publisher<std_msgs::msg::String>(topicFeedInfo, 10);
    threadPubFeedBackInfo = std::thread(&CRRobotRos2::pubFeedBackInfo, this);
    threadPubFeedBackInfo.detach();
}

void CRRobotRos2::pubFeedBackInfo()
{
    std::shared_ptr<RealTimeData> realTimeData;

    // 设置发布频率为100Hz
    rclcpp::Rate rate(100);

    while (rclcpp::ok())
    {
        realTimeData = commander_->getRealData();
        nlohmann::json root;

        root["len"] = realTimeData->len;
        root["digital_input_bits"] = realTimeData->digital_input_bits;
        root["digital_outputs"] = realTimeData->digital_outputs;
        root["robot_mode"] = realTimeData->robot_mode;
        root["controller_timer"] = realTimeData->controller_timer;
        root["run_time"] = realTimeData->run_time;
        root["test_value"] = realTimeData->test_value;
        root["safety_mode"] = realTimeData->safety_mode;
        root["speed_scaling"] = realTimeData->speed_scaling;
        root["linear_momentum_norm"] = realTimeData->linear_momentum_norm;
        root["v_main"] = realTimeData->v_main;
        root["v_robot"] = realTimeData->v_robot;
        root["i_robot"] = realTimeData->i_robot;
        root["program_state"] = realTimeData->program_state;
        root["safety_status"] = realTimeData->safety_status;
        std::vector<double> vecTransit; // vector 中转存取数组类型
        for (int i = 0; i < 3; i++)
        {
            vecTransit.push_back(realTimeData->tool_accelerometer_values[i]);
        }
        root["tool_accelerometer_values"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 3; i++)
        {
            vecTransit.push_back(realTimeData->elbow_position[i]);
        }
        root["elbow_position"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 3; i++)
        {
            vecTransit.push_back(realTimeData->elbow_velocity[i]);
        }
        root["elbow_velocity"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->q_target[i]);
        }
        root["q_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->qd_target[i]);
        }
        root["qd_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->qdd_target[i]);
        }
        root["qdd_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->i_target[i]);
        }
        root["i_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->m_target[i]);
        }
        root["m_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->q_actual[i]);
        }
        root["q_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->qd_actual[i]);
        }
       // cout << "Hello, world!" << endl;
        root["qd_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->i_actual[i]);
        }
        root["i_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->i_control[i]);
        }
        root["i_control"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->tool_vector_actual[i]);
        }
        root["tool_vector_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->TCP_speed_actual[i]);
        }
        root["TCP_speed_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->TCP_force[i]);
        }
        root["TCP_force"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->tool_vector_target[i]);
        }
        root["tool_vector_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->TCP_speed_target[i]);
        }
        root["TCP_speed_target"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->motor_temperatures[i]);
        }
        root["motor_temperatures"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->joint_modes[i]);
        }
        root["joint_modes"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->v_actual[i]);
        }
        root["v_actual"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 4; i++)
        {
            vecTransit.push_back(realTimeData->handtype[i]);
        }
        root["handtype"] = vecTransit;
        vecTransit.clear();

        root["userCoordinate"] = realTimeData->userCoordinate;
        root["toolCoordinate"] = realTimeData->toolCoordinate;
        root["isRunQueuedCmd"] = realTimeData->isRunQueuedCmd;
        root["isPauseCmdFlag"] = realTimeData->isPauseCmdFlag;
        root["velocityRatio"] = realTimeData->velocityRatio;
        root["accelerationRatio"] = realTimeData->accelerationRatio;
        root["jerkRatio"] = realTimeData->jerkRatio;
        root["xyzVelocityRatio"] = realTimeData->xyzVelocityRatio;
        root["rVelocityRatio"] = realTimeData->rVelocityRatio;
        root["xyzAccelerationRatio"] = realTimeData->xyzAccelerationRatio;
        root["rAccelerationRatio"] = realTimeData->rAccelerationRatio;
        root["xyzJerkRatio"] = realTimeData->xyzJerkRatio;
        root["rJerkRatio"] = realTimeData->rJerkRatio;
        root["BrakeStatus"] = realTimeData->BrakeStatus;
        root["EnableStatus"] = realTimeData->EnableStatus;
        root["DragStatus"] = realTimeData->DragStatus;
        root["RunningStatus"] = realTimeData->RunningStatus;
        root["ErrorStatus"] = realTimeData->ErrorStatus;
        root["JogStatus"] = realTimeData->JogStatus;
        root["RobotType"] = realTimeData->RobotType;
        root["DragButtonSignal"] = realTimeData->DragButtonSignal;
        root["EnableButtonSignal"] = realTimeData->EnableButtonSignal;
        root["RecordButtonSignal"] = realTimeData->RecordButtonSignal;
        root["ReappearButtonSignal"] = realTimeData->ReappearButtonSignal;
        root["JawButtonSignal"] = realTimeData->JawButtonSignal;
        root["SixForceOnline"] = realTimeData->SixForceOnline;
        root["CollisionStates"] = realTimeData->CollisionStates;
        root["ArmApproachState"] = realTimeData->ArmApproachState;
        root["J4ApproachState"] = realTimeData->J4ApproachState;
        root["J5ApproachState"] = realTimeData->J5ApproachState;
        root["J6ApproachState"] = realTimeData->J6ApproachState;
        root["vibrationDisZ"] = realTimeData->vibrationDisZ;
        root["currentCommandId"] = realTimeData->currentCommandId;

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->m_actual[i]);
        }
        root["m_actual"] = vecTransit;
        vecTransit.clear();

        root["load"] = realTimeData->load;
        root["centerX"] = realTimeData->centerX;
        root["centerY"] = realTimeData->centerY;
        root["centerZ"] = realTimeData->centerZ;

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->user[i]);
        }
        root["user"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->tool[i]);
        }
        root["tool"] = vecTransit;
        vecTransit.clear();

        root["TraceIndex"] = realTimeData->TraceIndex; // 1296 ~ 1303 轨迹复现索引 （未实现）

        for (int i = 0; i < 6; i++)
        {
            vecTransit.push_back(realTimeData->SixForceValue[i]);
        }
        root["SixForceValue"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 4; i++)
        {
            vecTransit.push_back(realTimeData->TargetQuaternion[i]);
        }
        root["TargetQuaternion"] = vecTransit;
        vecTransit.clear();

        for (int i = 0; i < 4; i++)
        {
            vecTransit.push_back(realTimeData->ActualQuaternion[i]);
        }
        root["ActualQuaternion"] = vecTransit;
        vecTransit.clear();
        root["AutoManualMode"] = realTimeData->AutoManualMode; // 1416 ~ 1417 手自动模式 0: 未开启 1: manual 2:auto
        std::string feedBackVecStr = root.dump();

        std_msgs::msg::String msgFeedInfo;
        msgFeedInfo.data = feedBackVecStr;
        kPublisherInfo->publish(msgFeedInfo);
        rate.sleep();
    }
}

void CRRobotRos2::execute_action(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request,
                                 std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Response> response)
{
    std::cout << "execute_action" << std::endl;
}

void CRRobotRos2::goalHandle()
{
    std::cout << "goalHandle" << std::endl;
}

void CRRobotRos2::getErrorID(std::vector<int> &vec)
{

    // 创建服务客户端
    std::string name = kRobotName + "/dobot_bringup_ros2/srv/GeterrorID";
    kClientGeterror = this->create_client<dobot_msgs_v4::srv::GetErrorID>(name);
    // 创建请求消息
    auto request = std::make_shared<dobot_msgs_v4::srv::GetErrorID::Request>();

    while (!kClientGeterror->service_is_ready())
    {
        if (!rclcpp::ok())
        {
            RCLCPP_ERROR(this->get_logger(), "Interrupted while waiting for the service. Exiting.");
            return;
        }
        //RCLCPP_INFO(this->get_logger(), "service not available, waiting again...");
    }

    auto result = kClientGeterror->async_send_request(request);

    // Wait for the result.
    auto self_shared_ptr = shared_from_this();
    // if (rclcpp::spin_until_future_complete(self_shared_ptr, result) ==
    //     rclcpp::FutureReturnCode::SUCCESS)
    // {
    //     for (auto i : result.get()->error_id)
    //     {
    //         vec.push_back(i);
    //         RCLCPP_INFO(this->get_logger(), "Sum: %d", result.get()->error_id[0]);
    //     }
    // }
    // else
    // {
    //     RCLCPP_ERROR(this->get_logger(), "Failed to call service add_two_ints");
    // }
}

void CRRobotRos2::backendTask()
{
    uint16_t robot_mode = commander_->getRobotMode();
    static uint16_t last_robot_mode_ = 0;
    if (robot_mode == 9 && last_robot_mode_ != 9)
    {
        std::vector<int> errorIdVec;
        this->getErrorID(errorIdVec);
        for (auto i : errorIdVec)
        {
            RCLCPP_ERROR(this->get_logger(), "errorId: %d", i);
        }
    }
    last_robot_mode_ = robot_mode;
}

void CRRobotRos2::getJointState(double *point)
{
    commander_->getCurrentJointStatus(point);
}

bool CRRobotRos2::isEnable() const
{
    return commander_->isEnable();
}

bool CRRobotRos2::isConnected() const
{
    return commander_->isConnected();
}

void CRRobotRos2::getToolVectorActual(double *val)
{
    commander_->getToolVectorActual(val);
}

/*
 *----------------------------------------------------------------------------------------------------------------------
 *                                                  dashboard
 *----------------------------------------------------------------------------------------------------------------------
 */

bool CRRobotRos2::EnableRobot(const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::EnableRobot::Response> response)
{
    return commander_->callRosService(parseTool::parserenableRobotRequest2String(request), response->res);
}

bool CRRobotRos2::DisableRobot(const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::DisableRobot::Response> response)
{
    return commander_->callRosService(parseTool::parserdisableRobotRequest2String(request), response->res);
}

bool CRRobotRos2::ClearError(const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::ClearError::Response> response)
{
    return commander_->callRosService(parseTool::parserclearErrorRequest2String(request), response->res);
}

bool CRRobotRos2::SpeedFactor(const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::SpeedFactor::Response> response)
{
    return commander_->callRosService(parseTool::parserspeedFactorRequest2String(request), response->res);
}

bool CRRobotRos2::User(const std::shared_ptr<dobot_msgs_v4::srv::User::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::User::Response> response)
{
    return commander_->callRosService(parseTool::parseruserRequest2String(request), response->res);
}

bool CRRobotRos2::Tool(const std::shared_ptr<dobot_msgs_v4::srv::Tool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Tool::Response> response)
{
    return commander_->callRosService(parseTool::parsertoolRequest2String(request), response->res);
}

bool CRRobotRos2::RobotMode(const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RobotMode::Response> response)
{
    return commander_->callRosService_f(parseTool::parserrobotModeRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::SetPayload(const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::SetPayload::Response> response)
{
    return commander_->callRosService(parseTool::parsersetPayloadRequest2String(request), response->res);
}

bool CRRobotRos2::DO(const std::shared_ptr<dobot_msgs_v4::srv::DO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DO::Response> response)
{
    return commander_->callRosService(parseTool::parserDORequest2String(request), response->res);
}

bool CRRobotRos2::DOInstant(const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DOInstant::Response> response)
{
    return commander_->callRosService(parseTool::parserDOInstantRequest2String(request), response->res);
}

bool CRRobotRos2::ToolDO(const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolDO::Response> response)
{
    return commander_->callRosService(parseTool::parsertoolDORequest2String(request), response->res);
}

bool CRRobotRos2::ToolDOInstant(const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::ToolDOInstant::Response> response)
{
    return commander_->callRosService(parseTool::parsertoolDOInstantRequest2String(request), response->res);
}

bool CRRobotRos2::AO(const std::shared_ptr<dobot_msgs_v4::srv::AO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AO::Response> response)
{
    return commander_->callRosService(parseTool::parserAORequest2String(request), response->res);
}

bool CRRobotRos2::AOInstant(const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AOInstant::Response> response)
{
    return commander_->callRosService(parseTool::parserAOInstantRequest2String(request), response->res);
}

bool CRRobotRos2::AccJ(const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AccJ::Response> response)
{
    return commander_->callRosService(parseTool::parseraccJRequest2String(request), response->res);
}

bool CRRobotRos2::AccL(const std::shared_ptr<dobot_msgs_v4::srv::AccL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AccL::Response> response)
{
    return commander_->callRosService(parseTool::parseraccLRequest2String(request), response->res);
}

bool CRRobotRos2::VelJ(const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::VelJ::Response> response)
{
    return commander_->callRosService(parseTool::parservelJRequest2String(request), response->res);
}

bool CRRobotRos2::VelL(const std::shared_ptr<dobot_msgs_v4::srv::VelL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::VelL::Response> response)
{
    return commander_->callRosService(parseTool::parservelLRequest2String(request), response->res);
}

bool CRRobotRos2::CP(const std::shared_ptr<dobot_msgs_v4::srv::CP::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CP::Response> response)
{
    return commander_->callRosService(parseTool::parsercpRequest2String(request), response->res);
}

bool CRRobotRos2::PowerOn(const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::PowerOn::Response> response)
{
    return commander_->callRosService(parseTool::parserpowerOnRequest2String(request), response->res);
}

bool CRRobotRos2::RunScript(const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::RunScript::Response> response)
{
    return commander_->callRosService(parseTool::parserrunScriptRequest2String(request), response->res);
}

bool CRRobotRos2::Stop(const std::shared_ptr<dobot_msgs_v4::srv::Stop::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Stop::Response> response)
{
    return commander_->callRosService(parseTool::parserstopRequest2String(request), response->res);
}

bool CRRobotRos2::Pause(const std::shared_ptr<dobot_msgs_v4::srv::Pause::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Pause::Response> response)
{
    return commander_->callRosService(parseTool::parserpauseRequest2String(request), response->res);
}

bool CRRobotRos2::Continue(const std::shared_ptr<dobot_msgs_v4::srv::Continue::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Continue::Response> response)
{
    return commander_->callRosService(parseTool::parserContinueRequest2String(request), response->res);
}

bool CRRobotRos2::SetCollisionLevel(const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Request> request,
                                    const std::shared_ptr<dobot_msgs_v4::srv::SetCollisionLevel::Response> response)
{
    return commander_->callRosService(parseTool::parsersetCollisionLevelRequest2String(request), response->res);
}

bool CRRobotRos2::EnableSafeSkin(const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Request> request,
                                 const std::shared_ptr<dobot_msgs_v4::srv::EnableSafeSkin::Response> response)
{
    return commander_->callRosService(parseTool::parserEnableSafeSkinRequest2String(request), response->res);
}

bool CRRobotRos2::SetSafeSkin(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::SetSafeSkin::Response> response)
{
    return commander_->callRosService(parseTool::parserSetSafeSkinRequest2String(request), response->res);
}

bool CRRobotRos2::GetStartPose(const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::GetStartPose::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetStartPoseRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::StartPath(const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StartPath::Response> response)
{
    return commander_->callRosService(parseTool::parserStartPathRequest2String(request), response->res);
}

bool CRRobotRos2::PositiveKin(const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::PositiveKin::Response> response)
{
    return commander_->callRosService(parseTool::parserPositiveKinRequest2String(request), response->res);
}

bool CRRobotRos2::InverseKin(const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::InverseKin::Response> response)
{
    return commander_->callRosService(parseTool::parserInverseKinRequest2String(request), response->res);
}

bool CRRobotRos2::GetAngle(const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetAngle::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetAngleRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetPose(const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetPose::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetPoseRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::EmergencyStop(const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::EmergencyStop::Response> response)
{
    return commander_->callRosService(parseTool::parserEmergencyStopRequest2String(request), response->res);
}

bool CRRobotRos2::ModbusRTUCreate(const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Request> request,
                                  const std::shared_ptr<dobot_msgs_v4::srv::ModbusRTUCreate::Response> response)
{
    return commander_->callRosService(parseTool::parserModbusRTUCreateRequest2String(request), response->res);
}

bool CRRobotRos2::ModbusCreate(const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::ModbusCreate::Response> response)
{
    return commander_->callRosService(parseTool::parserModbusCreateRequest2String(request), response->res);
}

bool CRRobotRos2::ModbusClose(const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::ModbusClose::Response> response)
{
    return commander_->callRosService(parseTool::parserModbusCloseRequest2String(request), response->res);
}

bool CRRobotRos2::GetInBits(const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInBits::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetInBitsRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetInRegs(const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetInRegs::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetInRegsRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetCoils(const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetCoils::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetCoilsRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::SetCoils(const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetCoils::Response> response)
{
    return commander_->callRosService(parseTool::parserSetCoilsRequest2String(request), response->res);
}

bool CRRobotRos2::GetHoldRegs(const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::GetHoldRegs::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetHoldRegsRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::SetHoldRegs(const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::SetHoldRegs::Response> response)
{
    return commander_->callRosService(parseTool::parserSetHoldRegsRequest2String(request), response->res);
}

bool CRRobotRos2::GetErrorID(const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::GetErrorID::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetErrorIDRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::DI(const std::shared_ptr<dobot_msgs_v4::srv::DI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DI::Response> response)
{
    return commander_->callRosService(parseTool::parserDIRequest2String(request), response->res);
}

bool CRRobotRos2::ToolDI(const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolDI::Response> response)
{
    return commander_->callRosService(parseTool::parserToolDIRequest2String(request), response->res);
}

bool CRRobotRos2::AI(const std::shared_ptr<dobot_msgs_v4::srv::AI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::AI::Response> response)
{
    return commander_->callRosService(parseTool::parserAIRequest2String(request), response->res);
}

bool CRRobotRos2::ToolAI(const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ToolAI::Response> response)
{
    return commander_->callRosService(parseTool::parserToolAIRequest2String(request), response->res);
}

bool CRRobotRos2::DIGroup(const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DIGroup::Response> response)
{
    return commander_->callRosService(parseTool::parserDIGroupRequest2String(request), response->res);
}

bool CRRobotRos2::DOGroup(const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::DOGroup::Response> response)
{
    return commander_->callRosService(parseTool::parserdoGroupRequest2String(request), response->res);
}

bool CRRobotRos2::BrakeControl(const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::BrakeControl::Response> response)
{
    return commander_->callRosService(parseTool::parserbrakeControlRequest2String(request), response->res);
}

bool CRRobotRos2::StartDrag(const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StartDrag::Response> response)
{
    return commander_->callRosService(parseTool::parserstartDragRequest2String(request), response->res);
}

bool CRRobotRos2::StopDrag(const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::StopDrag::Response> response)
{
    return commander_->callRosService(parseTool::parserStopDragRequest2String(request), response->res);
}

bool CRRobotRos2::DragSensivity(const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::DragSensivity::Response> response)
{
    return commander_->callRosService(parseTool::parserDragSensivityRequest2String(request), response->res);
}

bool CRRobotRos2::GetDO(const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetDO::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetDORequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetAO(const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::GetAO::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetAORequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetDOGroup(const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::GetDOGroup::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetDOGroupRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::SetTool485(const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Request> request,
                             const std::shared_ptr<dobot_msgs_v4::srv::SetTool485::Response> response)
{
    return commander_->callRosService(parseTool::parserSetTool485Request2String(request), response->res);
}

bool CRRobotRos2::SetSafeWallEnable(const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Request> request,
                                    const std::shared_ptr<dobot_msgs_v4::srv::SetSafeWallEnable::Response> response)
{
    return commander_->callRosService(parseTool::parserSetSafeWallEnableRequest2String(request), response->res);
}

bool CRRobotRos2::SetToolPower(const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::SetToolPower::Response> response)
{
    return commander_->callRosService(parseTool::parserSetToolPowerRequest2String(request), response->res);
}

bool CRRobotRos2::SetToolMode(const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::SetToolMode::Response> response)
{
    return commander_->callRosService(parseTool::parserSetToolModeRequest2String(request), response->res);
}

bool CRRobotRos2::SetBackDistance(const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Request> request,
                                  const std::shared_ptr<dobot_msgs_v4::srv::SetBackDistance::Response> response)
{
    return commander_->callRosService(parseTool::parserSetBackDistanceRequest2String(request), response->res);
}

bool CRRobotRos2::SetPostCollisionMode(const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Request> request,
                                       const std::shared_ptr<dobot_msgs_v4::srv::SetPostCollisionMode::Response> response)
{
    return commander_->callRosService(parseTool::parserSetPostCollisionModeRequest2String(request), response->res);
}

bool CRRobotRos2::SetUser(const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetUser::Response> response)
{
    return commander_->callRosService(parseTool::parserSetUserRequest2String(request), response->res);
}

bool CRRobotRos2::SetTool(const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::SetTool::Response> response)
{
    return commander_->callRosService(parseTool::parserSetToolRequest2String(request), response->res);
}

bool CRRobotRos2::CalcUser(const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CalcUser::Response> response)
{
    return commander_->callRosService(parseTool::parserCalcUserRequest2String(request), response->res);
}

bool CRRobotRos2::CalcTool(const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::CalcTool::Response> response)
{
    return commander_->callRosService(parseTool::parserCalcToolRequest2String(request), response->res);
}

bool CRRobotRos2::GetInputBool(const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::GetInputBool::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetInputboolRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetInputInt(const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::GetInputInt::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetInputIntRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetInputFloat(const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::GetInputFloat::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetInputFloatRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetOutputBool(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::GetOutputBool::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetOutputboolRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetOutputInt(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::GetOutputInt::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetOutputIntRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::GetOutputFloat(const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Request> request,
                                 const std::shared_ptr<dobot_msgs_v4::srv::GetOutputFloat::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetOutputFloatRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::SetOutputBool(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Request> request,
                                const std::shared_ptr<dobot_msgs_v4::srv::SetOutputBool::Response> response)
{
    return commander_->callRosService(parseTool::parserSetOutputboolRequest2String(request), response->res);
}

bool CRRobotRos2::SetOutputInt(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::SetOutputInt::Response> response)
{
    return commander_->callRosService(parseTool::parserSetOutputIntRequest2String(request), response->res);
}

bool CRRobotRos2::SetOutputFloat(const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Request> request,
                                 const std::shared_ptr<dobot_msgs_v4::srv::SetOutputFloat::Response> response)
{
    return commander_->callRosService(parseTool::parserSetOutputFloatRequest2String(request), response->res);
}

bool CRRobotRos2::MovJ(const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovJ::Response> response)
{
    return commander_->callRosService_f(parseTool::parsermovJRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::MovL(const std::shared_ptr<dobot_msgs_v4::srv::MovL::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovL::Response> response)
{
    return commander_->callRosService_f(parseTool::parsermovLRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::MovLIO(const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovLIO::Response> response)
{
    return commander_->callRosService(parseTool::parserMovLIORequest2String(request), response->res);
}

bool CRRobotRos2::MovJIO(const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MovJIO::Response> response)
{
    return commander_->callRosService(parseTool::parserMovJIORequest2String(request), response->res);
}

bool CRRobotRos2::Arc(const std::shared_ptr<dobot_msgs_v4::srv::Arc::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Arc::Response> response)
{
    return commander_->callRosService(parseTool::parserArcRequest2String(request), response->res);
}

bool CRRobotRos2::Circle(const std::shared_ptr<dobot_msgs_v4::srv::Circle::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::Circle::Response> response)
{
    return commander_->callRosService(parseTool::parserCircleRequest2String(request), response->res);
}

bool CRRobotRos2::MoveJog(const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::MoveJog::Response> response)
{
    return commander_->callRosService(parseTool::parsermoveJogRequest2String(request), response->res);
}

bool CRRobotRos2::StopMoveJog(const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::StopMoveJog::Response> response)
{
    return commander_->callRosService(parseTool::parserStopMoveJogRequest2String(request), response->res);
}

bool CRRobotRos2::RelMovJTool(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::RelMovJTool::Response> response)
{
    return commander_->callRosService(parseTool::parserRelMovJToolRequest2String(request), response->res);
}

bool CRRobotRos2::RelMovLTool(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::RelMovLTool::Response> response)
{
    return commander_->callRosService(parseTool::parserRelMovLToolRequest2String(request), response->res);
}

bool CRRobotRos2::RelMovJUser(const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::RelMovJUser::Response> response)
{
    return commander_->callRosService(parseTool::parserRelMovJUserRequest2String(request), response->res);
}

bool CRRobotRos2::RelMovLUser(const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Request> request,
                              const std::shared_ptr<dobot_msgs_v4::srv::RelMovLUser::Response> response)
{
    return commander_->callRosService(parseTool::parserRelMovLUserRequest2String(request), response->res);
}

bool CRRobotRos2::RelJointMovJ(const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Request> request,
                               const std::shared_ptr<dobot_msgs_v4::srv::RelJointMovJ::Response> response)
{
    return commander_->callRosService(parseTool::parserrelJointMovJRequest2String(request), response->res);
}

bool CRRobotRos2::GetCurrentCommandId(const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Request> request,
                                      const std::shared_ptr<dobot_msgs_v4::srv::GetCurrentCommandId::Response> response)
{
    return commander_->callRosService_f(parseTool::parserGetCurrentCommandIdRequest2String(request), response->res,response->robot_return);
}

bool CRRobotRos2::ServoJ(const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ServoJ::Response> response)
{
    return commander_->callRosService(parseTool::parserServoJRequest2String(request), response->res);
}

bool CRRobotRos2::ServoP(const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Request> request, const std::shared_ptr<dobot_msgs_v4::srv::ServoP::Response> response)
{
    return commander_->callRosService(parseTool::parserServoPRequest2String(request), response->res);
}
