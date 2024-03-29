cmake_minimum_required(VERSION 3.10.2)
project(cr_description)
find_package(ament_cmake REQUIRED)

ament_package()

install(DIRECTORY urdf meshes config DESTINATION share/${PROJECT_NAME})
