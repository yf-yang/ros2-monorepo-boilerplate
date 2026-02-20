#include <gtest/gtest.h>

#include "demo_cpp_node/message_builder.hpp"

TEST(MessageBuilderTest, BuildsMessageForInitialCount) {
  EXPECT_EQ(demo_cpp_node::build_publish_message(0), "hello from cpp #0");
}

TEST(MessageBuilderTest, BuildsMessageForArbitraryCount) {
  EXPECT_EQ(demo_cpp_node::build_publish_message(42), "hello from cpp #42");
}
