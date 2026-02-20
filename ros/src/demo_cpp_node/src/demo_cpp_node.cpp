// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

#include <chrono>
#include <functional>
#include <memory>

#include "demo_cpp_node/message_builder.hpp"
#include "interfaces/msg/greeting.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;
using Greeting = interfaces::msg::Greeting;

// Default queue size for ROS2 publishers and subscribers
constexpr std::size_t kDefaultQueueSize = 10;

class DemoCppNode : public rclcpp::Node {
 public:
  DemoCppNode() : Node("demo_cpp_node") {
    publisher_ = create_publisher<Greeting>("/cpp_to_py", kDefaultQueueSize);
    subscriber_ = create_subscription<Greeting>("/py_to_cpp", kDefaultQueueSize,
                                                // NOLINTNEXTLINE(performance-unnecessary-value-param)
                                                [this](const Greeting::SharedPtr msg) -> void { on_message(msg); });
    timer_ = create_wall_timer(1s, [this]() -> void { publish_message(); });
  }

 private:
  void publish_message() {
    Greeting msg;
    msg.sequence = static_cast<uint32_t>(publish_count_);
    msg.sender = get_name();
    msg.content = demo_cpp_node::build_publish_message(publish_count_++);
    publisher_->publish(msg);
    RCLCPP_INFO(get_logger(), "published to /cpp_to_py: [#%u] '%s'", msg.sequence, msg.content.c_str());
  }

  void on_message(const Greeting::SharedPtr& msg) const {
    RCLCPP_INFO(get_logger(), "received on /py_to_cpp from '%s': [#%u] '%s'", msg->sender.c_str(), msg->sequence,
                msg->content.c_str());
  }

  rclcpp::Publisher<Greeting>::SharedPtr publisher_;
  rclcpp::Subscription<Greeting>::SharedPtr subscriber_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t publish_count_{0};
};

auto main(int argc, char* argv[]) -> int {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DemoCppNode>());
  rclcpp::shutdown();
  return 0;
}