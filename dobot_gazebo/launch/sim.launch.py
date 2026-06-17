import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource


def generate_launch_description():
    # Gazebo + arm spawn + ros2_control
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('dobot_gazebo'), 'launch'), '/gazebo_moveit.launch.py']),
    )

    # MoveIt move_group + rviz
    moveit = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([os.path.join(
            get_package_share_directory('dobot_moveit'), 'launch'), '/moveit_gazebo.launch.py']),
    )

    return LaunchDescription([
        gazebo,
        moveit,
    ])
