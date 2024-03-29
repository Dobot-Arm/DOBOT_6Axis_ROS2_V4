cmake_minimum_required(VERSION 3.16)
project(dobot_msgs_v4)

# Default to C99
if (NOT CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 99)
endif ()

# Default to C++17
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif ()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -fdiagnostics-color=always)
endif ()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(builtin_interfaces REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

set(msg_files
        "msg/RobotStatus.msg"
        "msg/ToolVectorActual.msg"
        )

set(srv_files
       "srv/EnableRobot.srv"
       "srv/DisableRobot.srv"
       "srv/ClearError.srv"
       "srv/SpeedFactor.srv"
       "srv/User.srv"
       "srv/Tool.srv"
       "srv/RobotMode.srv"
       "srv/SetPayload.srv"
       "srv/DO.srv"
       "srv/DOInstant.srv"
       "srv/ToolDO.srv"
       "srv/ToolDOInstant.srv"
       "srv/AO.srv"
       "srv/AOInstant.srv"
       "srv/AccJ.srv"
       "srv/AccL.srv"
       "srv/VelJ.srv"
       "srv/VelL.srv"
       "srv/CP.srv"
       "srv/PowerOn.srv"
       "srv/RunScript.srv"
       "srv/Stop.srv"
       "srv/Pause.srv"
       "srv/Continue.srv"
       "srv/PositiveKin.srv"
       "srv/InverseKin.srv"
       "srv/SetCollisionLevel.srv"
       "srv/GetAngle.srv"
       "srv/GetPose.srv"
       "srv/EmergencyStop.srv"
       "srv/ModbusRTUCreate.srv"
       "srv/ModbusCreate.srv"
       "srv/ModbusClose.srv"
       "srv/GetInBits.srv"
       "srv/GetInRegs.srv"
       "srv/GetCoils.srv"
       "srv/SetCoils.srv"
       "srv/GetHoldRegs.srv"
       "srv/SetHoldRegs.srv"
       "srv/SetSafeSkin.srv"
       "srv/MovJ.srv"
       "srv/MovL.srv"
       "srv/RelJointMovJ.srv"
       "srv/MoveJog.srv"
       "srv/StopMoveJog.srv"
       "srv/DOGroup.srv"
       "srv/BrakeControl.srv"
       "srv/StartDrag.srv"
       "srv/EnableSafeSkin.srv"
       "srv/GetStartPose.srv"
       "srv/StartPath.srv"
       "srv/InverseSolution.srv"
       "srv/GetErrorID.srv"
       "srv/DI.srv"
       "srv/ToolDI.srv"
       "srv/AI.srv"
       "srv/ToolAI.srv"
       "srv/DIGroup.srv"
       "srv/StopDrag.srv"
       "srv/DragSensivity.srv"
       "srv/GetDO.srv"
       "srv/GetAO.srv"
       "srv/GetDOGroup.srv"
       "srv/SetTool485.srv"
       "srv/SetSafeWallEnable.srv"
       "srv/SetToolPower.srv"
       "srv/SetToolMode.srv"
       "srv/SetBackDistance.srv"
       "srv/SetPostCollisionMode.srv"
       "srv/SetUser.srv"
       "srv/SetTool.srv"
       "srv/CalcUser.srv"
       "srv/CalcTool.srv"
       "srv/GetInputBool.srv"
       "srv/GetInputInt.srv"
       "srv/GetInputFloat.srv"
       "srv/GetOutputBool.srv"
       "srv/GetOutputInt.srv"
       "srv/GetOutputFloat.srv"
       "srv/SetOutputBool.srv"
       "srv/SetOutputInt.srv"
       "srv/SetOutputFloat.srv"
       "srv/MovLIO.srv"
       "srv/MovJIO.srv"
       "srv/Arc.srv"
       "srv/Circle.srv"
       "srv/RelMovJTool.srv"
       "srv/RelMovLTool.srv"
       "srv/RelMovJUser.srv"
       "srv/RelMovLUser.srv"
       "srv/GetCurrentCommandId.srv"
       "srv/ServoJ.srv"
       "srv/ServoP.srv"
       "srv/TcpDashboard.srv"
        )

rosidl_generate_interfaces(${PROJECT_NAME}
        ${msg_files} ${srv_files}
        DEPENDENCIES
        builtin_interfaces
        std_msgs
        )

ament_export_dependencies(rosidl_default_generators)
ament_export_dependencies(rosidl_default_runtime)

ament_package()
