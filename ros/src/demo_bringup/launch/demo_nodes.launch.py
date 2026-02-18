from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description() -> LaunchDescription:
    return LaunchDescription(
        [
            Node(
                package="demo_cpp_node",
                executable="demo_cpp_node",
                name="demo_cpp_node",
                output="screen",
            ),
            Node(
                package="demo_py_node",
                executable="demo_py_node",
                name="demo_py_node",
                output="screen",
            ),
        ]
    )
