from setuptools import find_packages, setup

package_name = 'servo_action'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='dobot',
    maintainer_email='dobot@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
        'action_move_server    = servo_action.action_move_server:main',
        'action_move_client    = servo_action.action_move_client:main',
        'action_move_server_reality    = servo_action.action_move_server_reality:main',
        'action_move_client_reality    = servo_action.action_move_client_reality:main',
        'Joint_Position    = servo_action.Joint_Position:main',
        ],
    },
)
