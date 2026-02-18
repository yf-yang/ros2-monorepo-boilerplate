#include "demo_cpp_node/message_builder.hpp"

#include <string>

namespace demo_cpp_node {

std::string build_publish_message(std::size_t publish_count) {
  return "hello from cpp #" + std::to_string(publish_count);
}

} // namespace demo_cpp_node
