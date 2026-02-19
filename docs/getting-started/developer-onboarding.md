---
title: 开发者上手
description: 开发者进入容器并开始日常开发的最小步骤
read_when: 首次进入开发环境或需要查阅日常开发流程时
---

# 开发者上手

## 进入开发环境

1. 安装 VS Code 与 [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) 扩展。
2. 用 VS Code 打开本仓库，`Cmd/Ctrl+Shift+P` → **Dev Containers: Reopen in Container**。
3. 等待 devcontainer 初始化完成。
4. 打开 `dev.code-workspace` 文件，并点击右下角的 **Open Workspace** 按钮。
5. 打开 integrated terminal，确认 pixi 虚拟环境已自动激活（终端提示符应包含环境标识）。
6. 执行 `pnpm i`。

## ROS 开发

### 修改接口

修改 `ros/src/interfaces` 后，生成类型文件：

```bash
pixi run ros:build:interfaces
```

### 修改 ROS 包

```bash
pixi run ros:build <package-name>
```

### 代码质量检查（可选）

日常开发中 check 不是必须的，但 **PR 前必须通过**：

```bash
pixi run check <package-name>   # format + lint + typecheck + cspell + test
pixi run copyright:check        # 版权头检查
```

全量检查：

```bash
pixi run check:all
pixi run copyright:check
```
