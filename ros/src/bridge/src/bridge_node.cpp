// Copyright 2026 Dev Team
//
// Licensed under the Apache License, Version 2.0.

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <mcap_vendor/mcap/writer.hpp>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/timestamp.pb.h>

#include <zenoh.hxx>

#include "bridge/topic_message.pb.h"
#include "foxglove/Log.pb.h"
#include "interfaces/msg/greeting.hpp"
#include "rcl_interfaces/msg/log.hpp"
#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;
using Greeting = interfaces::msg::Greeting;
using RosLog = rcl_interfaces::msg::Log;
using ProtoTopicMessage = bridge::v1::TopicMessage;
using FoxgloveLog = foxglove::Log;

namespace {

// ROS 2 log levels → foxglove.Log.Level
foxglove::Log_Level ros_log_level_to_foxglove(uint8_t ros_level) {
  // rcl_interfaces/msg/Log: DEBUG=10, INFO=20, WARN=30, ERROR=40, FATAL=50
  switch (ros_level) {
    case 10:
      return foxglove::Log_Level_DEBUG;
    case 20:
      return foxglove::Log_Level_INFO;
    case 30:
      return foxglove::Log_Level_WARNING;
    case 40:
      return foxglove::Log_Level_ERROR;
    case 50:
      return foxglove::Log_Level_FATAL;
    default:
      return foxglove::Log_Level_UNKNOWN;
  }
}

std::string make_session_id() {
  auto now = std::chrono::system_clock::now();
  auto epoch_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch())
                      .count();
  return std::to_string(epoch_ms);
}

uint64_t wall_clock_ns() {
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count());
}

// Build a FileDescriptorSet containing the given descriptor's file and all
// transitive dependencies. This is required for MCAP protobuf schemas.
void collect_file_dependencies(
    const google::protobuf::FileDescriptor* file,
    google::protobuf::FileDescriptorSet& fd_set,
    std::unordered_set<std::string>& visited) {
  if (!file || visited.count(std::string(file->name()))) return;
  visited.insert(std::string(file->name()));
  for (int i = 0; i < file->dependency_count(); ++i) {
    collect_file_dependencies(file->dependency(i), fd_set, visited);
  }
  file->CopyTo(fd_set.add_file());
}

template <typename ProtoMsg>
std::string build_file_descriptor_set() {
  google::protobuf::FileDescriptorSet fd_set;
  std::unordered_set<std::string> visited;
  collect_file_dependencies(
      ProtoMsg::descriptor()->file(), fd_set, visited);
  std::string out;
  fd_set.SerializeToString(&out);
  return out;
}

}  // namespace

class BridgeNode : public rclcpp::Node {
 public:
  BridgeNode() : Node("bridge_node"), msg_seq_(0) {
    session_id_ = make_session_id();
    init_mcap();
    init_zenoh();
    init_subscriptions();

    timer_ = create_wall_timer(
        1s, std::bind(&BridgeNode::on_timer, this));

    RCLCPP_INFO(get_logger(), "bridge started, session=%s mcap=%s",
                session_id_.c_str(), mcap_path_.c_str());
  }

  ~BridgeNode() override {
    zenoh_pub_.reset();
    zenoh_session_.reset();
    file_writer_.close();
  }

 private:
  // ── Schema / Channel registration ──────────────────────────────

  struct ChannelInfo {
    mcap::ChannelId file_channel_id;
    std::string schema_name;
    std::string fds_bytes;
    std::string mcap_topic;
  };

  // Register a protobuf schema + MCAP channel pair.
  // Returns the index into channels_.
  template <typename ProtoMsg>
  size_t register_channel(const std::string& mcap_topic) {
    std::string schema_name(ProtoMsg::descriptor()->full_name());
    std::string fds = build_file_descriptor_set<ProtoMsg>();

    // File writer schema + channel
    mcap::Schema schema(schema_name, "protobuf",
                        std::string_view(fds));
    file_writer_.addSchema(schema);

    mcap::Channel channel(mcap_topic, "protobuf", schema.id);
    file_writer_.addChannel(channel);

    size_t idx = channels_.size();
    channels_.push_back({channel.id, schema_name, fds, mcap_topic});
    return idx;
  }

  // ── Subscription helper ────────────────────────────────────────

  template <typename RosMsg, typename ProtoMsg>
  void add_topic(
      const std::string& ros_topic, const std::string& mcap_topic,
      const rclcpp::QoS& qos,
      std::function<ProtoMsg(const typename RosMsg::SharedPtr&)> converter) {
    size_t ch_idx = register_channel<ProtoMsg>(mcap_topic);

    auto sub = create_subscription<RosMsg>(
        ros_topic, qos,
        [this, ch_idx, converter = std::move(converter)](
            const typename RosMsg::SharedPtr msg) {
          ProtoMsg proto = converter(msg);
          std::string serialized;
          proto.SerializeToString(&serialized);
          uint64_t ts = wall_clock_ns();

          // Write to file MCAP directly
          mcap::Message mcap_msg;
          mcap_msg.channelId = channels_[ch_idx].file_channel_id;
          mcap_msg.sequence = msg_seq_++;
          mcap_msg.logTime = ts;
          mcap_msg.publishTime = ts;
          mcap_msg.dataSize = serialized.size();
          mcap_msg.data =
              reinterpret_cast<const std::byte*>(serialized.data());
          auto status = file_writer_.write(mcap_msg);
          if (!status.ok()) {
            RCLCPP_WARN(get_logger(), "mcap file write error: %s",
                        status.message.c_str());
          }

          // Buffer for network mini-MCAP
          {
            std::lock_guard<std::mutex> lock(buf_mutex_);
            pending_.push_back(
                {ch_idx, std::move(serialized), ts});
          }
        });

    subscriptions_.push_back(sub);
  }

  // ── Initialization ─────────────────────────────────────────────

  void init_mcap() {
    mcap_path_ = "demo_" + session_id_ + ".mcap";

    mcap::McapWriterOptions opts("protobuf");
    // Very large chunk size so chunks are flushed only by the timer
    opts.chunkSize = 1024ULL * 1024 * 1024;  // 1 GiB
    opts.compression = mcap::Compression::Zstd;
    auto status = file_writer_.open(mcap_path_, opts);
    if (!status.ok()) {
      RCLCPP_ERROR(get_logger(), "failed to open mcap: %s",
                   status.message.c_str());
    }
  }

  void init_zenoh() {
    auto config = zenoh::Config::create_default();
    zenoh_session_.emplace(zenoh::Session::open(std::move(config)));

    auto pub_opts = zenoh::Session::PublisherOptions::create_default();
    pub_opts.congestion_control = Z_CONGESTION_CONTROL_DROP;

    zenoh_pub_.emplace(zenoh_session_->declare_publisher(
        zenoh::KeyExpr("bridge/mcap"), std::move(pub_opts)));

    RCLCPP_INFO(get_logger(), "zenoh publisher ready on bridge/mcap");
  }

  void init_subscriptions() {
    // QoS compatible with default (reliable, depth 10) publishers
    rclcpp::QoS data_qos = rclcpp::SystemDefaultsQoS();
    data_qos.keep_all();

    // /rosout uses RELIABLE + TRANSIENT_LOCAL
    rclcpp::QoS rosout_qos(1000);
    rosout_qos.reliable();
    rosout_qos.transient_local();

    // Greeting topics → TopicMessage
    auto greeting_converter =
        [this](const Greeting::SharedPtr& msg) -> ProtoTopicMessage {
      ProtoTopicMessage proto;
      proto.set_sender(msg->sender);
      proto.set_sequence(msg->sequence);
      proto.set_content(msg->content);
      proto.set_timestamp_ns(wall_clock_ns());
      proto.set_source_namespace(get_namespace());
      return proto;
    };

    add_topic<Greeting, ProtoTopicMessage>(
        "/cpp_to_py", "/cpp_to_py", data_qos,
        [this, greeting_converter](
            const Greeting::SharedPtr& msg) -> ProtoTopicMessage {
          auto proto = greeting_converter(msg);
          proto.set_topic("/cpp_to_py");
          return proto;
        });

    add_topic<Greeting, ProtoTopicMessage>(
        "/py_to_cpp", "/py_to_cpp", data_qos,
        [this, greeting_converter](
            const Greeting::SharedPtr& msg) -> ProtoTopicMessage {
          auto proto = greeting_converter(msg);
          proto.set_topic("/py_to_cpp");
          return proto;
        });

    // /rosout → foxglove.Log
    add_topic<RosLog, FoxgloveLog>(
        "/rosout", "/rosout", rosout_qos,
        [](const RosLog::SharedPtr& msg) -> FoxgloveLog {
          FoxgloveLog proto;
          auto* ts = proto.mutable_timestamp();
          ts->set_seconds(msg->stamp.sec);
          ts->set_nanos(static_cast<int32_t>(msg->stamp.nanosec));
          proto.set_level(ros_log_level_to_foxglove(msg->level));
          proto.set_message(msg->msg);
          proto.set_name(msg->name);
          proto.set_file(msg->file);
          proto.set_line(msg->line);
          return proto;
        });
  }

  // ── Timer: flush file chunk + publish network mini-MCAP ────────

  void on_timer() {
    // Flush file writer's current chunk to disk
    file_writer_.closeLastChunk();

    // Grab pending messages
    std::vector<PendingMessage> msgs;
    {
      std::lock_guard<std::mutex> lock(buf_mutex_);
      msgs.swap(pending_);
    }
    if (msgs.empty()) return;

    // Build a self-contained mini-MCAP in memory
    std::ostringstream oss;
    mcap::McapWriter net_writer;
    mcap::McapWriterOptions opts("protobuf");
    opts.compression = mcap::Compression::Zstd;
    net_writer.open(oss, opts);

    // Register only active schemas + channels in the mini-MCAP.
    // IDs may differ from the file writer, so build a mapping.
    std::vector<mcap::ChannelId> net_channel_ids(channels_.size());
    // Track which schemas have been registered (by schema_name)
    std::unordered_map<std::string, mcap::SchemaId> net_schema_ids;

    std::unordered_set<size_t> active_indices;
    for (const auto& pm : msgs) {
      active_indices.insert(pm.channel_idx);
    }

    for (size_t i = 0; i < channels_.size(); ++i) {
      if (active_indices.find(i) == active_indices.end()) continue;
      const auto& ch = channels_[i];
      mcap::SchemaId sid;
      auto it = net_schema_ids.find(ch.schema_name);
      if (it != net_schema_ids.end()) {
        sid = it->second;
      } else {
        mcap::Schema schema(ch.schema_name, "protobuf",
                            std::string_view(ch.fds_bytes));
        net_writer.addSchema(schema);
        sid = schema.id;
        net_schema_ids[ch.schema_name] = sid;
      }

      mcap::Channel channel(ch.mcap_topic, "protobuf", sid);
      net_writer.addChannel(channel);
      net_channel_ids[i] = channel.id;
    }

    // Write messages
    for (const auto& pm : msgs) {
      mcap::Message mcap_msg;
      mcap_msg.channelId = net_channel_ids[pm.channel_idx];
      mcap_msg.sequence = 0;
      mcap_msg.logTime = pm.timestamp;
      mcap_msg.publishTime = pm.timestamp;
      mcap_msg.dataSize = pm.data.size();
      mcap_msg.data =
          reinterpret_cast<const std::byte*>(pm.data.data());
      auto write_status = net_writer.write(mcap_msg);
      if (!write_status.ok()) {
        RCLCPP_WARN(get_logger(), "mini-mcap write error: %s",
                    write_status.message.c_str());
      }
    }

    net_writer.close();

    // Publish via zenoh
    std::string bytes = oss.str();
    RCLCPP_DEBUG(get_logger(), "published mini-mcap: %zu bytes, %zu msgs",
                 bytes.size(), msgs.size());
    zenoh_pub_->put(zenoh::Bytes(std::move(bytes)));
  }

  // ── Data types ─────────────────────────────────────────────────

  struct PendingMessage {
    size_t channel_idx;
    std::string data;
    uint64_t timestamp;
  };

  // Session
  std::string session_id_;
  uint32_t msg_seq_;

  // File MCAP
  std::string mcap_path_;
  mcap::McapWriter file_writer_;

  // Channel registry (shared between file + network writers)
  std::vector<ChannelInfo> channels_;

  // Pending messages for network
  std::mutex buf_mutex_;
  std::vector<PendingMessage> pending_;

  // Subscriptions (prevent GC)
  std::vector<rclcpp::SubscriptionBase::SharedPtr> subscriptions_;

  // Zenoh network transport
  std::optional<zenoh::Session> zenoh_session_;
  std::optional<zenoh::Publisher> zenoh_pub_;

  // ROS interfaces
  rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char* argv[]) {
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<BridgeNode>());
  rclcpp::shutdown();
  return 0;
}
