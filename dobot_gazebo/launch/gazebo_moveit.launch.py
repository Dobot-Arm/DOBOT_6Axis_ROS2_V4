import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, IncludeLaunchDescription, RegisterEventHandler
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
import xacro


def generate_launch_description():
    name = os.getenv("DOBOT_TYPE")
    robot_name_in_model = f'{name}_robot'
    package_name = 'cra_description'
    urdf_name = f"{name}_robot.xacro"

    # Launch Gazebo (gz sim). empty.sdf ships with a sun, ground plane and the
    # Physics/SceneBroadcaster/UserCommands system plugins that gz_ros2_control needs.
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch'), '/gz_sim.launch.py']),
        launch_arguments={'gz_args': '-r empty.sdf'}.items(),
    )

    cra_description_path = os.path.join(
        get_package_share_directory(package_name))

    xacro_file = os.path.join(cra_description_path,
                              'urdf',
                              urdf_name)

    doc = xacro.parse(open(xacro_file))
    xacro.process_doc(doc)
    robot_description_config = doc.toxml()
    robot_description = {'robot_description': robot_description_config, 'use_sim_time': True}

    node_robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[robot_description]
    )

    spawn_entity = Node(package='ros_gz_sim', executable='create',
                        arguments=['-topic', 'robot_description',
                                   '-name', robot_name_in_model],
                        output='screen')

    # Bridge gz /clock so ROS nodes share simulation time.
    clock_bridge = Node(package='ros_gz_bridge', executable='parameter_bridge',
                        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'],
                        output='screen')

    # The controller_manager is hosted inside the gz_ros2_control system plugin, so
    # controllers can only be loaded once the robot has been spawned into Gazebo.
    # # 关节状态发布器
    load_joint_state_controller = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             'joint_state_broadcaster'],
        output='screen'
    )

    # # 路径执行控制器
    load_joint_trajectory_controller = ExecuteProcess(
        cmd=['ros2', 'control', 'load_controller', '--set-state', 'active',
             f'{name}_group_controller'],
        output='screen'
    )

    close_evt1 = RegisterEventHandler(
            event_handler=OnProcessExit(
                target_action=spawn_entity,
                on_exit=[load_joint_state_controller],
            )
    )
    close_evt2 = RegisterEventHandler(
            event_handler=OnProcessExit(
                target_action=load_joint_state_controller,
                on_exit=[load_joint_trajectory_controller],
            )
    )

    return LaunchDescription([
      close_evt1,
      close_evt2,
      gazebo,
      clock_bridge,
      node_robot_state_publisher,
      spawn_entity
    ])
