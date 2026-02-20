---
title: "Bridge Node (C++)"
description: Bridge 节点的职责、双路 MCAP 架构（本地落盘 + 网络 mini-MCAP）、Zenoh 发布策略与 ROS 订阅配置
read_when: 需要理解 Bridge 节点的工作原理、修改其订阅/发布逻辑，或排查数据录制与传输问题时
---

# Bridge Node (C++)

## 定位

Bridge 节点是 ROS 与上位机之间的数据桥梁。它订阅 ROS topic，将消息转为 Protobuf 后同时写入本地 MCAP 文件和通过 Zenoh 网络发送给上位机。

## 数据流

```
ROS Topics ──→ Bridge Node ──┬──→ 本地 MCAP 文件（连续录制）
                             └──→ Zenoh (bridge/mcap)（定时 mini-MCAP）
```

## 双路 MCAP 架构

### 本地文件录制

- 文件名：`demo_<session_id>.mcap`（`session_id` 为启动时的毫秒级时间戳）
- chunk 大小设为 1 GiB，仅由定时器触发 `closeLastChunk()` 落盘
- 压缩：Zstd
- 每条消息携带全局递增 `sequence`、`logTime`、`publishTime`（均为 wall-clock 纳秒）

### 网络 mini-MCAP

每 1 秒触发一次定时器，将缓冲区中的消息打包为自包含的 mini-MCAP：

1. 从线程安全的 `pending_` 队列中取出本周期的全部消息
2. 在内存中创建新的 `McapWriter`，仅注册本周期活跃的 Schema/Channel（ID 与文件侧独立）
3. 写入消息，Zstd 压缩
4. 通过 Zenoh publisher 发布到 `bridge/mcap`

**设计要点：**

- 拥塞控制为 `DROP`——网络质量差时直接丢弃 chunk，不阻塞不重传
- 每个 mini-MCAP 自包含完整的 Schema + Channel + Message，接收端无需依赖历史状态即可独立解码
- 丢失的 chunk 不影响其他 chunk 的完整性；chunk 内部可以观察到多个节点间的消息转发 trace

## Schema / Channel 注册

`register_channel<ProtoMsg>(mcap_topic)` 模板方法：

1. 调用 `build_file_descriptor_set<ProtoMsg>()` 收集 proto 的 FileDescriptorSet（含所有传递依赖）
2. 向文件 writer 注册 Schema（encoding = `"protobuf"`，data = serialized FileDescriptorSet）和 Channel
3. 将 `ChannelInfo`（file_channel_id, schema_name, fds_bytes, mcap_topic）存入 `channels_`

这使得 MCAP 文件和 mini-MCAP 都包含完整的 protobuf schema 描述，兼容 Foxglove Studio 的 protobuf 解码。

## 当前订阅配置

| ROS Topic | MCAP Topic | Proto Schema | QoS |
|-----------|------------|--------------|-----|
| `/cpp_to_py` | `/cpp_to_py` | `bridge.TopicMessage` | reliable, keep_all |
| `/py_to_cpp` | `/py_to_cpp` | `bridge.TopicMessage` | reliable, keep_all |
| `/rosout` | `/rosout` | `foxglove.Log` | reliable, transient_local, depth=1000 |

## 新增订阅

简要步骤：

1. 定义 proto schema 并生成代码（详见 [Proto 工作流](/workflow/proto-and-mcap)）
2. 在 `init_subscriptions()` 中调用 `add_topic<RosMsg, ProtoMsg>(ros_topic, mcap_topic, qos, converter)`
3. `converter` 负责 ROS 消息 → Protobuf 的字段映射

## 关键文件

| 文件 | 说明 |
|------|------|
| `ros/src/bridge/src/bridge_node.cpp` | 节点完整实现 |
| `ros/src/bridge/CMakeLists.txt` | 构建配置，链接 proto、mcap、zenoh |
| `ros/src/bridge/package.xml` | ROS 包元数据与依赖声明 |
| `proto/bridge/topic_message.proto` | Bridge 自定义 proto |
| `proto/foxglove/Log.proto` | Foxglove Log proto |
