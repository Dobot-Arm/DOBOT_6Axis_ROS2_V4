from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    """MoveIt 标准演示（ros2_control），等价于主分支的 moveit_demo.launch.py。

    - 包含 {robot_type}_moveit/launch/demo.launch.py（MoveIt Setup Assistant 生成）
    - 走 ros2_control 控制器 + RViz
    - 与 dobot_moveit/launch/moveit_demo.launch.py（无硬件虚拟演示，fake_demo_server）互为补充
    """
    name = os.getenv("DOBOT_TYPE", "cr5")
    package_name = f'{name}_moveit'
    urdf_name = "demo.launch.py"

    pkg_share = os.path.join(get_package_share_directory(package_name))

    moveit_model_path = os.path.join(pkg_share, 'launch', urdf_name)
    ld = LaunchDescription([
        SetEnvironmentVariable('ROS_AUTOMATIC_DISCOVERY_RANGE', 'LOCALHOST'),
    ])
    included_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(moveit_model_path)
    )
    ld.add_action(included_launch)

    return ld
