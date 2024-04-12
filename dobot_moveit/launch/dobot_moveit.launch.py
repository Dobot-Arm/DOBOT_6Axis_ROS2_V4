from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
import os
from ament_index_python.packages import get_package_share_directory
def generate_launch_description():
    mane = os.getenv("DOBOT_TYPE")
    package_name = f'{mane}_moveit'
    urdf_name = "demo.launch.py"

    pkg_share = os.path.join(get_package_share_directory(package_name))

    moveit_model_path = os.path.join(pkg_share,'launch',urdf_name)
    ld = LaunchDescription()
    included_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(moveit_model_path)
    )
    ld.add_action(included_launch)

    # 添加其他操作...
    return ld
