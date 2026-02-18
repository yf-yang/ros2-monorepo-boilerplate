cmake_minimum_required(VERSION 3.16)
project(demo_cpp_node)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(interfaces REQUIRED)

add_library(demo_cpp_node_message_builder src/message_builder.cpp)
target_compile_features(demo_cpp_node_message_builder PUBLIC cxx_std_17)
target_include_directories(demo_cpp_node_message_builder PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

add_executable(demo_cpp_node src/demo_cpp_node.cpp)
target_compile_features(demo_cpp_node PUBLIC cxx_std_17)
ament_target_dependencies(demo_cpp_node rclcpp interfaces)
target_link_libraries(demo_cpp_node demo_cpp_node_message_builder)

install(TARGETS demo_cpp_node demo_cpp_node_message_builder
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY include/ DESTINATION include)

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_message_builder test/test_message_builder.cpp)
  if(TARGET test_message_builder)
    target_link_libraries(test_message_builder demo_cpp_node_message_builder)
  endif()
endif()

ament_package()
