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

class DemoCppNode : public rclcpp::Node {
public:
  DemoCppNode() : Node("demo_cpp_node"), publish_count_(0) {
    publisher_ = create_publisher<Greeting>("/cpp_to_py", 10);
    subscriber_ = create_subscription<Greeting>(
        "/py_to_cpp", 10,
        std::bind(&DemoCppNode::on_message, this, std::placeholders::_1));
    timer_ =
        create_wall_timer(1s, std::bind(&DemoCppNode::publish_message, this));
  }

private:
  void publish_message() {
    Greeting msg;
    msg.sequence = static_cast<uint32_t>(publish_count_);
    msg.sender = get_name();
    msg.content = demo_cpp_node::build_publish_message(publish_count_++);
    publisher_->publish(msg);
    RCLCPP_INFO(get_logger(), "published to /cpp_to_py: [#%u] '%s'",
                msg.sequence, msg.content.c_str());
  }

  void on_message(const Greeting::SharedPtr msg) const {
    RCLCPP_INFO(get_logger(), "received on /py_to_cpp from '%s': [#%u] '%s'",
                msg->sender.c_str(), msg->sequence, msg->content.c_str());
  }

  rclcpp::Publisher<Greeting>::SharedPtr publisher_;
  rclcpp::Subscription<Greeting>::SharedPtr subscriber_;
  rclcpp::TimerBase::SharedPtr timer_;
  std::size_t publish_count_;
};

int main(int argc, char *argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DemoCppNode>());
  rclcpp::shutdown();
  return 0;
}