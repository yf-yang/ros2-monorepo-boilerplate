cmake_minimum_required(VERSION 3.16)
project(interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# To add a new message: place the .msg file under msg/<domain>/,
# then add its path here following the same pattern.
rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/demo/Greeting.msg"
)

ament_package()
