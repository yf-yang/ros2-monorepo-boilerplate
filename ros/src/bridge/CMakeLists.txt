cmake_minimum_required(VERSION 3.16)
project(bridge)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(interfaces REQUIRED)
find_package(rcl_interfaces REQUIRED)
find_package(mcap_vendor REQUIRED)
find_package(protobuf CONFIG REQUIRED)
find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)

set(PROTO_GEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../gen/cpp")

add_library(bridge_proto
  ${PROTO_GEN_DIR}/bridge/topic_message.pb.cc
  ${PROTO_GEN_DIR}/foxglove/Log.pb.cc
)
target_compile_features(bridge_proto PUBLIC cxx_std_17)
target_include_directories(bridge_proto PUBLIC ${PROTO_GEN_DIR})
target_link_libraries(bridge_proto PUBLIC protobuf::libprotobuf)

add_executable(bridge_node src/bridge_node.cpp)
target_compile_features(bridge_node PUBLIC cxx_std_17)
ament_target_dependencies(bridge_node
  rclcpp
  interfaces
  rcl_interfaces
)
target_link_libraries(bridge_node bridge_proto mcap zenohcxx::zenohc)

install(TARGETS bridge_node DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
