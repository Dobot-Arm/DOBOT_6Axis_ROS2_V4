from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.conditions import UnlessCondition
from launch.substitutions import LaunchConfiguration
import os


def generate_launch_description():
    robot_type = os.getenv("DOBOT_TYPE", "cr5")

    robot_type_arg = DeclareLaunchArgument(
        name='robot_type',
        default_value=robot_type,
        description='Robot type (e.g., cr5, cr10, cr12)'
    )

    timeout_arg = DeclareLaunchArgument(
        name='trajectory_execution_timeout',
        default_value='120.0',
        description='Trajectory execution timeout in seconds'
    )

    use_sim_arg = DeclareLaunchArgument(
        name='use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) time instead of system time'
    )

    # ServoJ 同步策略：extend_t(按计划发送并抬高 t) | advance_send(提前发送，t 不变)
    sync_mode_arg = DeclareLaunchArgument(
        name='servoj_sync_mode',
        default_value='extend_t',
        description='ServoJ sync mode: extend_t | advance_send'
    )

    # 提前量 = max(段时长 × servoj_lead_percent%, servoj_lead_min_sec)
    lead_percent_arg = DeclareLaunchArgument(
        name='servoj_lead_percent',
        default_value='5.0',
        description='Lead as percentage of segment time (5.0 = 5%)'
    )

    lead_min_arg = DeclareLaunchArgument(
        name='servoj_lead_min_sec',
        default_value='0.005',
        description='Lead absolute lower bound in seconds (0.005 = 5ms)'
    )

    return LaunchDescription([
        robot_type_arg,
        timeout_arg,
        use_sim_arg,
        sync_mode_arg,
        lead_percent_arg,
        lead_min_arg,
        Node(
            package='dobot_moveit',
            executable='action_move_server',
            parameters=[
                {'robot_type': LaunchConfiguration('robot_type')},
                {'trajectory_execution_timeout': LaunchConfiguration('trajectory_execution_timeout')},
                {'servoj_sync_mode': LaunchConfiguration('servoj_sync_mode')},
                {'servoj_lead_percent': LaunchConfiguration('servoj_lead_percent')},
                {'servoj_lead_min_sec': LaunchConfiguration('servoj_lead_min_sec')}
            ],
            output='screen',
            condition=UnlessCondition(LaunchConfiguration('use_sim_time')),
        ),
        Node(
            package='dobot_moveit',
            executable='joint_states',
            parameters=[{
                'publish_frequency': 50.0,
            }],
            output='screen',
            condition=UnlessCondition(LaunchConfiguration('use_sim_time')),
        ),
    ])