from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='dobot_moveit',
            executable='action_move_server',
        ),
        Node(
            package='dobot_moveit',
            executable='joint_states',
        ),
    ])