---
title: "pixi: 任务执行入口与环境管理"
description: pixi 在本仓库中的任务入口与 Python/C++ 环境职责
read_when: 需要新增或修改 pixi task、搜索或添加 conda 包、管理 Python/C++ 依赖，或理解 pixi 与 nx 的协作时
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
- 包级质量检查（经 nx 编排，带缓存）：`format`、`lint`、`typecheck`、`cspell`、`check`、`test`（均支持 `<package-name>` 和 `:all` 后缀）
- 文件级工具（直接调用，不经 nx）：`clang-format`、`clang-tidy`、`ruff-format`、`ruff-check`、`ty`、`oxfmt`、`oxlint`、`cspell:files`
- 版权头：`copyright`、`copyright:check`
- 文档：`list-docs`、`read-docs`、`doc-site`

其中包级任务会调用 `nx` 执行具体 target（见 `tooling/nx-dependency-graph-and-cache.md`）。

### 文件级任务命名约定

- 默认（无后缀）：只检查，不修改文件（如 `clang-format`、`ruff-format`）
- `:fix` 后缀：写入/修复（如 `clang-format:fix`、`ruff-format:fix`、`ruff-check:fix`）

## 搜索与查找包

添加新依赖前，需要先确认包在 conda-forge 中的名称和可用性。

### CLI 搜索：`pixi search`

```bash
pixi search <name>       # 精确搜索，显示版本、依赖等详情
pixi search '<glob>' -l 10  # 通配符模糊搜索，列出匹配的包名
```

`pixi search` 会自动使用项目 `pixi.toml` 中配置的 channel（本仓库为 robostack-jazzy + conda-forge 镜像），因此搜索结果与实际可安装的包一致。

### 网页搜索：prefix.dev

在 https://prefix.dev/channels/conda-forge 的搜索栏中输入关键词，适合不确定包名时进行浏览式探索。

### conda-forge 包命名惯例

conda-forge 中 C++ 库和 Python 包有不同的命名惯例，搜索时可据此区分：

| 类型 | 常见命名模式 | 示例 |
|------|-------------|------|
| C/C++ 库 | `lib<name>` | `libopencv`、`libprotobuf`、`libarrow` |
| C++ 库（旧惯例） | `<name>-cpp` | `grpc-cpp`、`boost-cpp` |
| Python 绑定 | `py<name>` 或 `py-<name>` | `pyarrow`、`py-opencv` |
| Python 包 | 直接用上游名称 | `numpy`、`flask` |

因此搜索同一个库的不同语言绑定时，可以用通配符覆盖多种命名：

```bash
pixi search 'opencv*' -l 10   # 列出 opencv、libopencv、py-opencv 等
pixi search 'lib<name>*'      # 侧重 C++ 库
pixi search 'py-<name>*'      # 侧重 Python 绑定
```

## 常用命令

```bash
# 查看 docs 元数据
pixi run list-docs

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
