cmake_minimum_required(VERSION 3.5)
project(cr_robot_ros2)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(dobot_msgs_v4 REQUIRED)
find_package(control_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)


include_directories(
        include
        ../../../install/dobot_msgs_v4/include
)


add_executable(${PROJECT_NAME}_node src/main.cpp src/cr_robot_ros2.cpp src/command.cpp src/tcp_socket.cpp src/parseTool.cpp src/trajectory_executor.cpp)

target_include_directories(${PROJECT_NAME}_node PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>) 

ament_target_dependencies(
  ${PROJECT_NAME}_node
  "rclcpp"
  "rclcpp_action"
  "std_msgs"
  "sensor_msgs"
  "dobot_msgs_v4"
  "control_msgs"
  "trajectory_msgs"
)

target_link_libraries(${PROJECT_NAME}_node 
        pthread
        )

install(TARGETS ${PROJECT_NAME}_node
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()

  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_trajectory_executor
    test/test_trajectory_executor.cpp
    src/trajectory_executor.cpp
    src/command.cpp
    src/tcp_socket.cpp
  )
  target_include_directories(test_trajectory_executor PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  ament_target_dependencies(test_trajectory_executor
    "rclcpp"
    "trajectory_msgs"
    "dobot_msgs_v4"
  )
  target_link_libraries(test_trajectory_executor pthread)
endif()
ament_package()
