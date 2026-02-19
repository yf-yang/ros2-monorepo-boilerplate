---
title: "nx: 依赖关系与缓存编排"
description: nx 在本仓库中的任务依赖编排与缓存策略
read_when: 需要新增或修改 nx target、排查缓存问题或调整任务依赖时
---

# nx：依赖关系与缓存编排

## 在本仓库里的定位

`nx` 负责：

1. 任务编排：统一执行各 package 的 target。
2. 依赖关系：通过 `dependsOn` 控制先后顺序。
3. 缓存：为可缓存任务提供复用能力，减少重复执行。

本仓库日常入口仍是 `pixi run ...`，通常不要求开发者直接操作 nx 原生命令。

## 关键配置文件

- `nx.json`：全局配置、target 默认缓存策略
- `ros/project.json`：ROS 工作区级任务（`build-dev`、`build-ci`、`lint`、`run-demo`）
- `ros/src/*/project.json`：各 package 的 `format/lint/typecheck/cspell/test` 等 target

## 依赖关系示例

- `demo_cpp_node:test` / `demo_py_node:test` 依赖 `ros_ws:build-dev`，确保测试前先可构建。
- `ros_ws:run-demo` 依赖 `ros_ws:build-dev`，确保运行前先完成构建。

## 缓存策略（来自 `nx.json`）

默认开启缓存：

- `format`
- `lint`
- `typecheck`
- `cspell`
- `test`

默认关闭缓存：

- `build-dev`
- `build-ci`
- `run-demo`

## 常见使用方式

```bash
# 通过 pixi 调用 nx 执行单包测试
pixi run test demo_py_node

# 通过 pixi 调用 nx 执行全量检查
pixi run check:all
```

## 官方文档

- Nx docs: https://nx.dev/docs
- Project configuration: https://nx.dev/reference/project-configuration
- Task caching: https://nx.dev/concepts/how-caching-works
