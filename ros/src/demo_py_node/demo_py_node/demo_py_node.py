from __future__ import annotations

import rclpy
from interfaces.msg import Greeting
from rclpy.node import Node


class DemoPyNode(Node):
    def __init__(self) -> None:
        super().__init__("demo_py_node")
        self._publish_count = 0
        self._publisher = self.create_publisher(Greeting, "/py_to_cpp", 10)
        self._subscriber = self.create_subscription(
            Greeting,
            "/cpp_to_py",
            self._on_message,
            10,
        )
        self._timer = self.create_timer(1.0, self._publish_message)

    def _publish_message(self) -> None:
        message = Greeting()
        message.sequence = self._publish_count
        message.sender = self.get_name()
        message.content = f"hello from python #{self._publish_count}"
        self._publish_count += 1
        self._publisher.publish(message)
        self.get_logger().info(
            f"published to /py_to_cpp: [#{message.sequence}] '{message.content}'"
        )

    def _on_message(self, message: Greeting) -> None:
        self.get_logger().info(
            f"received on /cpp_to_py from '{message.sender}': [#{message.sequence}] '{message.content}'"
        )


def main(args: list[str] | None = None) -> None:
    rclpy.init(args=args)
    node = DemoPyNode()
    try:
        rclpy.spin(node)
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
