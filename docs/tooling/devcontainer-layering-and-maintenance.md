---
title: devcontainer 分层与维护
description: devcontainer 两层结构、维护边界与 dev-base 构建流程
read_when: 需要修改 devcontainer 配置、调整分层边界或重建基础镜像时
---

# devcontainer 分层与维护

## 读者范围

这篇文档主要面向运维/维护者。普通开发者通常只需要“Reopen in Container”即可。

## 两层结构与关系

本仓库采用两层 devcontainer 结构：

1. 基础层（`devImage/.devcontainer`）
   - 产物是本机镜像 `dev-base`
   - 包含 devcontainer features 与不常变更的全局依赖
   - 例如：Node、Nx、Go、通用 shell 工具、全局 cspell、copywrite 等

2. 仓库层（`.devcontainer/devcontainer.json`）
   - 直接使用 `"image": "dev-base"`
   - 叠加本仓库专属配置（workspace/mounts/env/postCreate）与 VS Code 扩展

## 首次准备（本机）

先确保本机有 npm，然后[安装 devcontainer CLI](https://code.visualstudio.com/docs/devcontainers/devcontainer-cli#_npm-install)：

```bash
npm install -g @devcontainers/cli
```

## 构建基础镜像

在仓库根目录执行：

```bash
devcontainer build --workspace-folder ./devImage --image-name dev-base
```

该命令会读取 `devImage/.devcontainer/devcontainer.json` 与对应 Dockerfile，构建本机 `dev-base` 镜像。

## 何时改哪一层

改基础层（`devImage`）的典型场景：

- 多仓库共用的工具或 feature
- 低频变更、希望集中维护的全局依赖

改仓库层（`.devcontainer`）的典型场景：

- 本仓库独有的 VS Code 扩展
- 本仓库运行参数、挂载、环境变量
- 本仓库 postCreate/postStart 行为

## 推荐维护流程

1. 修改 `devImage/.devcontainer/*`。
2. 重新构建 `dev-base` 镜像。
3. 在仓库执行 Rebuild/Reopen Container without Cache 验证。
4. 如只改仓库层，通常不需要重建 `dev-base`，直接重建当前容器即可。

## 官方文档

- Dev Container spec: https://containers.dev/implementors/spec/
- Dev Containers CLI: https://github.com/devcontainers/cli
- VS Code Dev Containers: https://code.visualstudio.com/docs/devcontainers/containers
