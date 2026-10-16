import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import (DeclareLaunchArgument, IncludeLaunchDescription, ExecuteProcess,
                            RegisterEventHandler, SetEnvironmentVariable)
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PythonExpression
import xacro


def generate_launch_description():
    name = os.getenv("DOBOT_TYPE", "cr5")
    robot_name_in_model = f'{name}_robot'
    package_name = 'cra_description'
    urdf_name = f"{name}_robot.xacro"

    # Gazebo GUI is enabled by default. On machines without a GPU the GUI may fall
    # back to software rendering and consume high CPU; in that case launch with
    # gui:=false to run server-only (RViz still provides visualization).
    gui_arg = DeclareLaunchArgument('gui', default_value='true')
    gz_args = PythonExpression(
        ["'-r empty.sdf' if '", LaunchConfiguration('gui'), "' == 'true' else '-s -r empty.sdf'"]
    )

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch'), '/gz_sim.launch.py']),
        launch_arguments={'gz_args': gz_args}.items(),
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

    clock_bridge = Node(package='ros_gz_bridge', executable='parameter_bridge',
                        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'],
                        output='screen')

    # Use our custom controller_loader instead of the ros2_control spawner.
    # The spawner aborts on the first ok=false response, but gz_ros2_control
    # non-deterministically returns ok=false even when operations succeed.
    # controller_loader.py checks actual controller state via list_controllers
    # instead of trusting the ok field. It also waits for hardware interfaces
    # to be available before loading controllers, so no fixed delay is needed.
    spawn_controllers = ExecuteProcess(
        cmd=['ros2', 'run', 'dobot_gazebo', 'controller_loader.py', name],
        output='screen',
    )

    close_evt = RegisterEventHandler(
            event_handler=OnProcessExit(
                target_action=spawn_entity,
                on_exit=[spawn_controllers],
            )
    )

    return LaunchDescription([
      SetEnvironmentVariable('ROS_AUTOMATIC_DISCOVERY_RANGE', 'LOCALHOST'),
      gui_arg,
      close_evt,
      gazebo,
      clock_bridge,
      node_robot_state_publisher,
      spawn_entity
    ])
