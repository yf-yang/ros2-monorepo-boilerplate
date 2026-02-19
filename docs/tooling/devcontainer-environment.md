---
title: 开发环境
description: devcontainer 基础镜像、zsh 插件（别名、语法高亮、自动建议、目录跳转等）及 tab 补全配置
read_when: 需要了解容器内的 shell 环境、可用的别名与快捷命令，或确认基础镜像信息时
---
# 开发环境

## 基础镜像

本仓库的 devcontainer 基于 `mcr.microsoft.com/devcontainers/base:noble`，即 **Ubuntu 24.04 LTS (Noble Numbat)**。

C++ 依赖尽量通过 pixi 管理（详见 [pixi 文档](pixi-task-runner-and-python-cpp-env.md)），而非直接安装到系统中，以便后续迁移到其他基础镜像或平台时减少耦合。

## Shell 环境

默认 shell 为 **zsh**，并预装了 **Oh My Zsh** 框架。以下是启用的插件及其用途。

### 视觉增强

#### zsh-syntax-highlighting

在命令行输入时实时对命令进行语法高亮。正确的命令显示为绿色，错误的命令显示为红色，便于在按下回车前发现拼写错误。

#### zsh-autosuggestions

根据历史记录在光标右侧显示灰色的补全建议。按 `→` 键接受整条建议，按 `Ctrl+→` 接受一个单词。

### 命令与别名

#### aliases

提供 `als` 命令，用于查找和筛选当前所有已注册的 shell 别名。

```bash
als           # 列出所有别名
als git       # 筛选包含 "git" 的别名
als -g        # 仅列出全局别名
```

#### common-aliases

为常见命令预设大量缩写别名，例如：

| 别名 | 展开 | 说明 |
|------|------|------|
| `la` | `ls -lAh` | 列出所有文件（含隐藏），人类可读大小 |
| `l`  | `ls -lah` | 同上 |
| `..` | `cd ..` | 返回上级目录 |
| `...` | `cd ../..` | 返回上两级目录 |
| `H`  | `\| head` | 管道到 head |
| `G`  | `\| grep` | 管道到 grep |
| `dud` | `du -d 1 -h` | 当前目录下各子目录大小 |
| `fd` | `find . -type d -name` | 按名称查找目录 |
| `ff` | `find . -type f -name` | 按名称查找文件 |

使用 `als` 命令可查看完整列表。

#### git-commit

提供以 [Conventional Commits](https://www.conventionalcommits.org/) 规范快速生成 commit message 的命令：

```bash
gc feat "add new sensor driver"       # -> git commit -m "feat: add new sensor driver"
gc fix "correct imu calibration"      # -> git commit -m "fix: correct imu calibration"
gc docs "update architecture guide"   # -> git commit -m "docs: update architecture guide"
gc refactor "simplify tf listener"    # -> git commit -m "refactor: simplify tf listener"
gc chore "bump dependencies"          # -> git commit -m "chore: bump dependencies"
```

支持的 type 包括：`feat`、`fix`、`docs`、`style`、`refactor`、`perf`、`test`、`build`、`ci`、`chore`、`revert`。

也可以添加 scope：

```bash
gc feat "add lidar node" -s perception  # -> git commit -m "feat(perception): add lidar node"
```

#### history

提供搜索和查看命令历史的快捷命令：

```bash
h          # 显示命令历史（等同于 history）
hs <word>  # 在历史中搜索包含 <word> 的命令
hsi <word> # 同上，但忽略大小写
```

#### pre-commit

提供 [pre-commit](https://pre-commit.com/) 工具的常用别名：

```bash
prc    # pre-commit
prca   # pre-commit run --all-files
prcau  # pre-commit autoupdate
```

#### z

跟踪你访问过的目录，之后可以用关键词模糊跳转，无需输入完整路径：

```bash
cd /workspaces/ros/src/perception/lidar  # 先正常访问一次
cd /workspaces/ros/docs/tooling          # 再访问别的地方

z lidar       # 直接跳回 .../perception/lidar
z tooling     # 直接跳回 .../docs/tooling
z ros src     # 匹配包含 "ros" 和 "src" 的最常访问路径
```

`z` 会根据访问频率和最近访问时间（frecency）排序，越常去的目录优先级越高。

### Tab 补全

以下插件仅提供 tab 补全支持，不再赘述：

- **nvm** — Node.js 版本管理器补全
- **uv** — Python 包管理器补全
