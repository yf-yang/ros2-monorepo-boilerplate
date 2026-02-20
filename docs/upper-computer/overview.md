---
title: 上位机
description: 上位机的当前实现（demo_upper CLI）、live/replay 两种模式的工作原理，以及向 Electron 应用迁移的规划
read_when: 需要理解或修改上位机的数据接收与解码逻辑、运行 live/replay 命令，或规划上位机迁移时
---

# 上位机

## 当前状态

上位机目前是一个临时性的 Node.js/TypeScript CLI 工具（`demo_upper/`），用于验证 Bridge 节点的数据传输链路。未来将迁移到 Electron 应用。

## 运行模式

### Live 模式

实时接收 Bridge 节点通过 Zenoh 发送的 mini-MCAP 数据：

```bash
pixi run demo:upper:live                         # 默认 localhost:10000
pixi run demo:upper:live -- --host 192.168.1.10  # 指定远程主机
```

数据链路：

```
Bridge Node ──Zenoh──→ zenoh-bridge-remote-api ──WebSocket──→ demo_upper
                           (bridge/mcap)
```

处理流程：
1. 通过 `@eclipse-zenoh/zenoh-ts` 连接到 `zenoh-bridge-remote-api` 的 WebSocket 端口
2. 订阅 Zenoh key `bridge/mcap`
3. 收到 mini-MCAP 字节流后，使用 `McapStreamReader` 逐条解析 Schema → Channel → Message
4. 按 schema name 分发到 `DECODERS` 注册表进行 protobuf 反序列化
5. 打印解码后的消息

### Replay 模式

回放本地 MCAP 文件：

```bash
pixi run demo:upper:replay -- --file demo_1234567890.mcap
```

使用 `McapIndexedReader` 读取文件，支持索引访问。处理流程与 live 模式的消息解码逻辑相同。

## zenoh-bridge-remote-api

`@eclipse-zenoh/zenoh-ts` 需要通过 WebSocket 连接到 Zenoh 网络，这由 `zenoh-bridge-remote-api` 桥接进程提供。

```bash
./demo_upper/setup-bridge.sh  # 下载当前平台的 zenoh-bridge-remote-api 二进制
```

二进制安装到 `demo_upper/bin/zenoh-bridge-remote-api`，支持 Linux (x86_64/aarch64) 和 macOS (x86_64/arm64)。

## 消息解码机制

`decode.ts` 维护一个 schema name → decoder 的注册表：

```typescript
const DECODERS: Record<string, Decoder> = {
  "bridge.v1.TopicMessage": (data) => fromBinary(TopicMessageSchema, ...),
  "foxglove.Log": (data) => fromBinary(LogSchema, ...),
};
```

新增 schema 的支持方式详见 [Proto 工作流](/workflow/proto-and-mcap)。

## 关键文件

| 文件 | 说明 |
|------|------|
| `demo_upper/src/cli.ts` | CLI 入口，定义 live/replay 子命令 |
| `demo_upper/src/live.ts` | live 模式：Zenoh 订阅 + mini-MCAP 解析 |
| `demo_upper/src/replay.ts` | replay 模式：MCAP 文件回放 |
| `demo_upper/src/decode.ts` | protobuf 解码注册表与打印逻辑 |
| `demo_upper/setup-bridge.sh` | 下载 zenoh-bridge-remote-api |
| `demo_upper/generated/proto/` | buf 生成的 TypeScript proto 代码 |
| `demo_upper/package.json` | 依赖声明 |

## 未来方向

当前 CLI 为验证用途，后续将迁移至 Electron 应用，届时：
- `demo_upper/` 目录将被替换或重构
- 核心解码逻辑（decode.ts）和 proto 生成产物可复用
- live/replay 的数据处理管线将迁移到 Electron 进程中
