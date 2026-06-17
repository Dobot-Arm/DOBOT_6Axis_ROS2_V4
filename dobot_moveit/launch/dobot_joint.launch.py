from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
import os


def generate_launch_description():
    robot_type = os.getenv("DOBOT_TYPE", "cr5")

    # robot_type 参数
    robot_type_arg = DeclareLaunchArgument(
        name='robot_type',
        default_value=robot_type,
        description='Robot type (e.g., cr5, cr10, cr12)'
    )

    # trajectory_execution_timeout 参数（接收父 launch 传递的参数）
    timeout_arg = DeclareLaunchArgument(
        name='trajectory_execution_timeout',
        default_value='120.0',
        description='Trajectory execution timeout in seconds'
    )

    return LaunchDescription([
        robot_type_arg,
        timeout_arg,
        Node(
            package='dobot_moveit',
            executable='action_move_server',
            parameters=[
                {'robot_type': LaunchConfiguration('robot_type')},
                {'trajectory_execution_timeout': LaunchConfiguration('trajectory_execution_timeout')}
            ],
            output='screen',
        ),
    ])