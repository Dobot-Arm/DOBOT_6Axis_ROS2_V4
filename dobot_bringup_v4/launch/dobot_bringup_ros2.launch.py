from launch import LaunchDescription
from launch.actions import SetEnvironmentVariable
import launch_ros.actions
import json
import os

cur_path = os.path.split(os.path.realpath(__file__))[0] + '/'
cur_config_path = cur_path + '../config'
cur_json_path = os.path.join(cur_config_path, 'param.json')

# 读取 JSON 文件
with open(cur_json_path, 'r') as file:
    json_data = json.load(file)

robot_number = json_data["robot_number"]
current_robot = json_data['current_robot']
node_info = json_data["node_info"]


# ip_address = json_data["node_info"][current_robot -
#                                     1]["ip_address"]    # robot IP
# robot_type = json_data["node_info"][current_robot -
#                                     1]["robot_type"]  # robot type
trajectory_duration = json_data["node_info"][current_robot -
                                             1]["trajectory_duration"]
robot_node_name = json_data["node_info"][current_robot -
                                         1]["robot_node_name"]
ip_address = os.getenv("IP_address") or json_data["node_info"][current_robot - 1]["ip_address"]

robot_type = os.getenv("DOBOT_TYPE") or json_data["node_info"][current_robot - 1]["robot_type"]


dobot_ros2_params = [
    {"robot_ip_address": ip_address},
    {"robot_type": robot_type},
    {"trajectory_duration": trajectory_duration},
    {"robot_node_name": robot_node_name},
    {"robot_number": robot_number},
]


def generate_launch_description():

    return LaunchDescription([
        SetEnvironmentVariable('ROS_AUTOMATIC_DISCOVERY_RANGE', 'LOCALHOST'),
        launch_ros.actions.Node(
            package='dobot_bringup_v4',
            executable='dobot_bringup_v4_node',
            name=robot_node_name,
            output='screen',
            parameters=dobot_ros2_params
            # respawn=True
        ),
    ])
