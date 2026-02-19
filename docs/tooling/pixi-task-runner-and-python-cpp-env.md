---
title: "pixi: 任务执行入口与环境管理"
description: pixi 在本仓库中的任务入口与 Python/C++ 环境职责
read_when: 需要新增或修改 pixi task、管理 Python/C++ 依赖，或理解 pixi 与 nx 的协作时
---

# pixi：任务执行入口与 Python/C++ 环境管理

## 在本仓库里的定位

`pixi` 有两个核心职责：

1. 统一任务入口：开发者通过 `pixi run ...` 执行几乎所有工作流命令。
2. 管理依赖与环境：统一管理 ROS、Python、C++ 相关依赖与工具版本。

## 关键配置文件

- `pixi.toml`：workspace、feature、依赖与任务定义
- `pixi.lock`：依赖锁定结果
- `scripts/ros_activate.sh`：jazzy feature 的激活脚本

## 任务分层（本仓库约定）

- ROS 构建/运行：`ros:build*`、`ros:demo`
- 质量检查：`format*`、`lint*`、`typecheck*`、`cspell*`、`check*`
- 测试：`test*`
- 版权头：`copyright*`
- 文档：`read-docs`、`doc-site`

其中大量任务会调用 `nx` 执行具体 target（见 `tooling/nx-dependency-graph-and-cache.md`）。

## 常用命令

```bash
# 查看 docs 元数据
pixi run read-docs

# 构建整个 ROS 工作区
pixi run ros:build:all

# 对单个包执行质量检查
pixi run check <package-name>
```

## 开发体验相关约定

在 devcontainer 里打开终端后，pixi 环境会自动激活；默认 Python 解释器来自 `.pixi/envs/default/bin/python`。

## 官方文档

- pixi docs: https://pixi.prefix.dev/latest/
- pixi tasks: https://pixi.prefix.dev/latest/workspace/advanced_tasks/
