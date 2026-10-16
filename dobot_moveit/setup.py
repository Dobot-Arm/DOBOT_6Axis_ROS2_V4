from setuptools import find_packages, setup
import os
from glob import glob

package_name = 'dobot_moveit'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        (os.path.join('share', package_name, 'launch'), glob(os.path.join('launch', '*.launch.py'))),
        # 只安装 .py 文件：用 'scripts/*' 会把 scripts/__pycache__ 目录也当成数据文件导致构建失败
        (os.path.join('lib', package_name), glob(os.path.join('scripts', '*.py'))),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='FTX',
    maintainer_email='futingxing@dobot-robot.com',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        'joint_states    = dobot_moveit.joint_states:main',
        'action_move_server    = dobot_moveit.action_move_server:main',
        'fake_demo_server    = dobot_moveit.fake_demo_server:main',
        ],
    },
)
