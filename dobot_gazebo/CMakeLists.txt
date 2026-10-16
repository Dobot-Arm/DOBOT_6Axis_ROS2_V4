cmake_minimum_required(VERSION 3.5)
project(dobot_gazebo)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

install(DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
install(DIRECTORY worlds
  DESTINATION share/${PROJECT_NAME}
)
install(DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)
install(PROGRAMS
  dobot_gazebo/controller_loader.py
  DESTINATION lib/${PROJECT_NAME}
)
ament_package()
