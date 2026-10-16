from ament_index_python.packages import get_package_share_path
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, SetEnvironmentVariable
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
import os

def generate_launch_description():
    name = os.getenv("DOBOT_TYPE", "cr5")
    urdf_tutorial_path = get_package_share_path('dobot_rviz')
    default_model_path = urdf_tutorial_path / f'urdf/{name}_robot.urdf'
    default_rviz_config_path = urdf_tutorial_path / 'rviz/urdf.rviz'

    gui_arg = DeclareLaunchArgument(name='gui', default_value='false', choices=['true', 'false'],
                                    description='Flag to enable joint_state_publisher_gui')
    model_arg = DeclareLaunchArgument(name='model', default_value=str(default_model_path),
                                      description='Absolute path to robot urdf file')
    rviz_arg = DeclareLaunchArgument(name='rvizconfig', default_value=str(default_rviz_config_path),
                                     description='Absolute path to rviz config file')

    robot_description = ParameterValue(Command(['xacro ', LaunchConfiguration('model')]),
                                       value_type=str)

    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{
            'robot_description': robot_description,
        }]
    )

    # Depending on gui parameter, either launch joint_state_publisher or joint_state_publisher_gui
    joint_state_publisher_node = Node(
        package='joint_state_publisher',
        executable='joint_state_publisher',
        condition=UnlessCondition(LaunchConfiguration('gui'))
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

    # Restrict DDS to localhost only. Without this, another machine on the same
    # network running a CR5 bringup publishes /tf for the CR5 model. When this
    # launch loads a different model (e.g. cr3), RViz receives two conflicting
    # sets of transforms for the same frame names (local cr3 + remote cr5 with
    # different link lengths) and the model jumps up and down. cr5 is unaffected
    # because both transforms happen to match. localhost isolation stops the
    # remote TF from leaking in.
    return LaunchDescription([
        SetEnvironmentVariable('ROS_AUTOMATIC_DISCOVERY_RANGE', 'LOCALHOST'),
        gui_arg,
        model_arg,
        rviz_arg,
        joint_state_publisher_node,
        joint_state_publisher_gui_node,
        robot_state_publisher_node,
        rviz_node
    ])
