from ament_index_python.packages import get_package_share_path
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
import os


def generate_launch_description():
    name = os.getenv("DOBOT_TYPE", "cr5")
    urdf_tutorial_path = get_package_share_path('dobot_rviz')
    default_model_path = str(urdf_tutorial_path / f'urdf/{name}_robot.urdf')
    default_rviz_config_path = str(urdf_tutorial_path / 'rviz/urdf.rviz')

    gui_arg = DeclareLaunchArgument(
        name='gui', default_value='false', choices=['true', 'false'],
        description='Flag to enable joint_state_publisher_gui')
    model_arg = DeclareLaunchArgument(
        name='model', default_value=default_model_path,
        description='Absolute path to robot urdf file')
    rviz_arg = DeclareLaunchArgument(
        name='rvizconfig', default_value=default_rviz_config_path,
        description='Absolute path to rviz config file')
    robot_arg = DeclareLaunchArgument(
        name='live_hardware', default_value='false', choices=['true', 'false'],
        description='Set to true when connecting to a real robot via dobot_bringup')

    with open(default_model_path, 'r') as f:
        robot_description_content = f.read()

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description_content}],
        remappings=[('/joint_states', '/rsp_joint_states')]
    )

    # Standalone mode: no --robot flag
    joint_state_relay_node = Node(
        package='dobot_rviz',
        executable='joint_state_relay.py',
        condition=UnlessCondition(LaunchConfiguration('live_hardware')),
        arguments=[default_model_path]
    )

    # Real robot mode: adds --robot flag, subscribes to /joint_states_robot directly
    joint_state_relay_robot_node = Node(
        package='dobot_rviz',
        executable='joint_state_relay.py',
        name='joint_state_relay',
        condition=IfCondition(LaunchConfiguration('live_hardware')),
        arguments=[default_model_path, '--robot']
    )

    joint_state_publisher_gui_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        condition=IfCondition(LaunchConfiguration('gui'))
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', LaunchConfiguration('rvizconfig')],
    )

    return LaunchDescription([
        gui_arg,
        model_arg,
        rviz_arg,
        robot_arg,
        robot_state_publisher_node,
        joint_state_relay_node,
        joint_state_relay_robot_node,
        joint_state_publisher_gui_node,
        rviz_node
    ])
