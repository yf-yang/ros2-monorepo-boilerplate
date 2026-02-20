---
title: Proto 生成与 MCAP 通信工作流
description: Protobuf schema 定义、buf 代码生成（C++ / TypeScript）、MCAP 数据格式，以及新增 schema 到数据在上位机可视的端到端流程
read_when: 需要新增或修改 proto schema、理解 MCAP 数据格式、或排查 proto 生成与解码问题时
---

# Proto 生成与 MCAP 通信工作流

## 端到端流程

```
proto/ 下定义 .proto
        │
        ▼
  buf generate ──┬──→ C++ (.pb.h/.pb.cc) → ros/generated/proto/
                 └──→ TypeScript (.ts)    → demo_upper/generated/proto/
                          │                          │
                          ▼                          ▼
                 Bridge 节点写入 MCAP          上位机解码展示
```

## Proto 目录结构

```
proto/
├── buf.gen.yaml            # 代码生成配置
├── bridge/
│   └── topic_message.proto # Bridge 自定义 schema（package bridge.v1）
├── foxglove/
│   ├── Log.proto           # Foxglove 官方 schema（保持兼容）
│   ├── RawImage.proto
│   └── ...                 # 40+ foxglove schema
└── google/
    └── protobuf/
        └── timestamp.proto # Well-known types
```

- `bridge/`：项目自定义的 proto，按业务领域组织
- `foxglove/`：来自 Foxglove SDK 的官方 schema，尽量复用以保持与 Foxglove Studio 的兼容性

## buf 配置

**`buf.yaml`**（仓库根目录）：声明 `proto/` 为 module，启用 STANDARD lint 规则和 FILE 级 breaking change 检测。

**`proto/buf.gen.yaml`**：定义两个代码生成插件：

| 插件 | 输出目录 | 说明 |
|------|---------|------|
| `protoc_builtin: cpp` | `ros/generated/proto` | 本地 protoc，版本须与 pixi 管理的 libprotobuf ABI 匹配 |
| `remote: buf.build/bufbuild/es` | `demo_upper/generated/proto` | 远程 TS 插件，与 protoc 版本无耦合 |

## 代码生成命令

```bash
pixi run proto:gen   # buf generate --template proto/buf.gen.yaml
pixi run proto:lint  # buf lint
```

生成产物已纳入 `.gitignore`，需在构建前执行。

## MCAP 格式要点

MCAP 文件由以下记录组成：

1. **Schema**：encoding = `"protobuf"`，data = serialized `FileDescriptorSet`（含所有传递依赖的 `.proto` 文件描述符）
2. **Channel**：关联一个 Schema，指定 topic 名称
3. **Message**：关联一个 Channel，携带 timestamp、sequence、serialized protobuf bytes

Bridge 节点在注册 Channel 时自动构建 FileDescriptorSet，确保 MCAP 文件自描述。这使得 Foxglove Studio 可以直接打开 MCAP 文件并解码所有消息。

## Foxglove 兼容性

- `proto/foxglove/` 下的 schema 直接来自 Foxglove SDK，保持原始 package 名和字段定义
- Bridge 对 `/rosout` 的转换遵循 `foxglove.Log` 的字段语义（level 映射、timestamp 格式）
- 自定义 schema（如 `bridge.v1.TopicMessage`）在 Foxglove Studio 中以 raw protobuf 形式展示

## 新增 Schema 的完整步骤

### 1. 定义 Proto

在 `proto/` 下创建 `.proto` 文件，遵循 buf STANDARD lint 规则：

```protobuf
syntax = "proto3";
package mymodule.v1;

message MyMessage {
  string field_a = 1;
  uint64 timestamp_ns = 2;
}
```

### 2. 生成代码

```bash
pixi run proto:gen
```

产物：
- C++：`ros/generated/proto/mymodule/my_message.pb.h`、`.pb.cc`
- TS：`demo_upper/generated/proto/mymodule/my_message_pb.ts`

### 3. Bridge 侧接入

在 `bridge_node.cpp` 的 `init_subscriptions()` 中：

```cpp
#include "mymodule/my_message.pb.h"

add_topic<MyRosMsg, mymodule::v1::MyMessage>(
    "/my_ros_topic", "/my_mcap_topic", qos,
    [](const MyRosMsg::SharedPtr& msg) -> mymodule::v1::MyMessage {
      mymodule::v1::MyMessage proto;
      // field mapping ...
      return proto;
    });
```

同时在 `CMakeLists.txt` 中将新的 `.pb.cc` 文件加入 `bridge_proto` 库的源文件列表。

### 4. 上位机侧接入

在 `demo_upper/src/decode.ts` 的 `DECODERS` 注册表中添加解码器：

```typescript
import { MyMessageSchema } from "../generated/proto/mymodule/my_message_pb.ts";

const DECODERS: Record<string, Decoder> = {
  // ... existing decoders
  "mymodule.v1.MyMessage": (data) =>
    fromBinary(MyMessageSchema, new Uint8Array(data)),
};
```

并添加对应的 print 函数和 `decodeAndPrint` 分支。

## 关键文件

| 文件 | 说明 |
|------|------|
| `buf.yaml` | buf module 与 lint/breaking 配置 |
| `proto/buf.gen.yaml` | 代码生成插件配置 |
| `proto/bridge/*.proto` | Bridge 自定义 schema |
| `proto/foxglove/*.proto` | Foxglove 官方 schema |
| `ros/generated/proto/` | C++ 生成产物 |
| `demo_upper/generated/proto/` | TypeScript 生成产物 |
| `ros/src/bridge/src/bridge_node.cpp` | Bridge 节点（Schema 注册与消息写入） |
| `demo_upper/src/decode.ts` | 上位机解码注册表 |
