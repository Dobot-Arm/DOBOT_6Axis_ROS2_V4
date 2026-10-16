from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from moveit_configs_utils import MoveItConfigsBuilder
from ament_index_python.packages import get_package_share_directory
import os


def generate_launch_description():
    name = os.getenv("DOBOT_TYPE", "cr5")
    package_name = f'{name}_moveit'
    robot_name = f'{name}_robot'

    moveit_config = MoveItConfigsBuilder(robot_name, package_name=package_name).to_moveit_configs()

    # SRDF "home" positions for each robot type (joint1..joint6)
    HOME_POSITIONS = {
        'cr3':    [0.0,  0.6072, -1.7223, -0.2949,  1.6134, 0.0],
        'cr5':    [0.0,  0.4337, -1.4695, -0.2602,  1.7175, 0.0],
        'cr7':    [0.0,  0.5898, -1.4952, -0.5204,  1.5613, 0.0],
        'cr10':   [0.0,  0.3296, -1.3436, -0.3643,  1.6134, 0.0],
        'cr10af': [0.0,  0.3818,  1.1880, -0.5901, -1.5621, 0.0],
        'cr12':   [0.0,  0.5204, -1.4335, -0.5204,  1.7695, 0.0],
        'cr16':   [0.0,  0.3643, -1.4695, -0.4337,  1.7175, 0.0],
        'cr20':   [0.0,  0.7980, -1.9066, -0.1041,  1.6307, 0.0],
        'cr30h':  [0.0,  0.6592,  1.6185, -0.1041,  1.6307, 0.0],
        'nova2':  [0.0,  0.4684, -1.3719,  0.0000,  1.7695, 0.0],
        'nova5':  [0.0,  0.5378, -1.1869,  0.0000,  1.7785, 0.0],
        'me6':    [0.0,  0.5337,  0.7276,  0.3548, -1.5848, 0.0],
    }

    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time", default_value="false",
        description="Use simulation (Gazebo) time instead of system time"
    )

    ld = LaunchDescription([
        SetEnvironmentVariable('ROS_AUTOMATIC_DISCOVERY_RANGE', 'LOCALHOST'),
        use_sim_time_arg,
    ])

    # ── Virtual joint TFs ────────────────────────────────
    virtual_joints_launch = (
        moveit_config.package_path / "launch/static_virtual_joint_tfs.launch.py"
    )
    if virtual_joints_launch.exists():
        ld.add_action(IncludeLaunchDescription(
            PythonLaunchDescriptionSource(str(virtual_joints_launch)),
        ))

    # ── Robot State Publisher ────────────────────────────
    ld.add_action(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            str(moveit_config.package_path / "launch/rsp.launch.py")
        ),
    ))

    # ── MoveGroup (standard cr5_group_controller) ─────────
    ld.add_action(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            str(moveit_config.package_path / "launch/move_group.launch.py")
        ),
    ))

    # ── RViz ─────────────────────────────────────────────
    ld.add_action(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            str(moveit_config.package_path / "launch/moveit_rviz.launch.py")
        ),
    ))

    # ── Fake Demo Server ─────────────────────────────────
    # Single node that:
    #   1. Publishes /joint_states with proper timestamps
    #   2. Provides fake FollowJointTrajectory action server on
    #      {robot_type}_group_controller/follow_joint_trajectory
    #   3. Updates joint positions after each executed trajectory
    # This replaces ros2_control for the virtual demo.
    ld.add_action(Node(
        package='dobot_moveit',
        executable='fake_demo_server',
        parameters=[{
            'robot_type': name,
            'joint_names': ['joint1', 'joint2', 'joint3',
                           'joint4', 'joint5', 'joint6'],
            'initial_positions': HOME_POSITIONS.get(name, [0.0]*6),
            'publish_frequency': 50.0,
            'use_sim_time': LaunchConfiguration('use_sim_time'),
        }],
        output='screen',
    ))

    return ld
