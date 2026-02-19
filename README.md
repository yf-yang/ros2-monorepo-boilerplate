## AI 辅助开发

**请积极使用 AI 编码工具（Claude、Codex 等）辅助日常开发。** 本仓库已为 AI 提供了完整的上下文配置，它们能理解仓库结构、开发工作流和编码规范，可以高效地协助你完成编码、调试、重构等任务。

### 上下文配置

仓库根目录下有三个 AI 上下文文件（`CLAUDE.md` 和 `GEMINI.md` 是 `AGENTS.md` 的软链接）：

- **`AGENTS.md`** — 主文件，AI 工具启动时自动加载

这些文件中只放**最公共的信息**：仓库目录结构、编码规范、常用开发命令、文档体系说明等。AI 需要这些信息来理解仓库全貌并遵守基本约定。更深入的设计决策、架构细节、工具链配置指南等不放在这里（会让 AI 的初始上下文过于臃肿），而是维护在 `docs/` 下的各篇文档中。

### 与 docs/ 的协同

`docs/` 下的每篇文档都带有 front matter 元数据（`description` 和 `read_when`），AI 据此判断当前任务是否需要加载某篇文档的全文——只在相关时才读取，避免浪费上下文窗口。

这意味着：**维护好 `docs/` 中的文档就是在帮助 AI 更好地理解仓库。** 当你新增架构决策、工具链变更或开发规范时，写到 `docs/` 并配好 front matter，AI 就能在需要时自动找到。反过来，如果某个设计决策只存在于你的脑子里而没有写进文档，AI 就无法知道——所以请养成及时更新文档的习惯，这直接决定了 AI 辅助开发的质量。

关于 front matter 和 `docs/` 的详细说明见下方[文档](#文档)一节。

## 快速开始

### 1. 安装 VS Code 与 Dev Containers 扩展

本仓库使用 [Dev Container](https://containers.dev/) 作为开发环境——它是一个预配置好所有依赖（ROS2、编译器、Python、pixi 等）的 Docker 容器，确保每个开发者拿到完全一致的环境，无需手动安装任何依赖。

你需要：

- [VS Code](https://code.visualstudio.com/)
- [Dev Containers 扩展](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)
- 本机已安装 Docker（或兼容运行时）

### 2. 进入开发容器

用 VS Code 打开本仓库，`Cmd/Ctrl+Shift+P` → **Dev Containers: Reopen in Container**。

VS Code 会根据仓库中的 `.devcontainer/` 配置拉取/构建镜像并启动容器。首次启动需要较长时间，后续会使用缓存。

### 3. 切换到多根工作区

容器启动后，打开根目录下的 `dev.code-workspace`，点击右下角 **Open Workspace** 按钮。

这会让 VS Code 以多根工作区模式打开，各个子项目能获得独立的语言服务和配置。

### 4. 确认 pixi 环境

打开 VS Code integrated terminal，确认 `pixi` 虚拟环境已自动激活（终端提示符包含环境标识）。

[pixi](https://pixi.sh/) 是本仓库的**统一命令入口**。所有构建、测试、检查命令都通过 `pixi run <task>` 执行。它同时负责管理 Python 和 C++ 的依赖——你不需要手动安装 Python 包或系统库，pixi 会在虚拟环境中自动处理。打开 integrated terminal 时，pixi 环境会自动激活。

### 5. 安装 Node 依赖

```bash
pnpm i
```

本仓库的部分工具脚本（如 nx 任务编排、文档浏览工具）是用 JavaScript 编写的，因此需要安装 Node 依赖。

### 6. 验证环境

```bash
pixi run ros:build:interfaces
pixi run ros:build:all
pixi run ros:demo
```

`ros:demo` 正常启动即说明环境就绪。

## 工具链

### pixi

[pixi](https://pixi.sh/) 是本仓库的统一命令入口。所有开发者需要记住的只有 `pixi run <task>`。

pixi 的配置文件是仓库根目录下的 `pixi.toml`。你会在这个文件中做两类事情：

**定义任务**——`[tasks]` 节下声明可执行的命令，例如：

```toml
"ros:build" = { cmd = "colcon build --packages-select", cwd = "ros" }
"test" = "sh -c 'nx run $1:test' sh"
```

之后就可以通过 `pixi run ros:build <pkg>` 或 `pixi run test <pkg>` 调用。

**管理 C++/Python 依赖**——在 `[feature.*.dependencies]` 节下声明 conda 依赖。例如当前的 ROS Jazzy 环境声明了 `ros-jazzy-ros-core`、`cmake`、`ruff` 等。需要新的系统库或 Python 包时，在这里添加，pixi 会自动解析并安装到隔离的虚拟环境中，不需要手动 `pip install` 或 `apt install`。

### nx

[nx](https://nx.dev/) 是一个 monorepo 任务编排工具。你**不需要直接调用 nx**——所有 nx 任务都已封装在 `pixi run` 命令中。但理解它的作用有助于理解仓库的行为。

**依赖感知**——nx 知道包与包之间的依赖关系，会自动按正确顺序执行。例如 `demo_py_node` 的 `test` target 声明了对 `ros_ws` 的 `build-dev` 的依赖：

```json
"test": {
  "dependsOn": [{ "projects": ["ros_ws"], "target": "build-dev" }],
  ...
}
```

当你执行 `pixi run test demo_py_node` 时，nx 会先确保 `ros_ws:build-dev` 已完成，然后才运行测试。你不需要手动记住"先构建再测试"的顺序。

**缓存**——如果输入文件没有变化，nx 会跳过执行并直接返回上次的结果。这就是为什么有时候构建或检查会瞬间完成。哪些 target 启用了缓存在 `nx.json` 的 `targetDefaults` 中配置：

```json
"format": { "cache": true },
"lint": { "cache": true },
"test": { "cache": true },
"build-dev": { "cache": false }
```

每个包的 nx 配置在各自目录下的 `project.json` 中（如 `ros/src/demo_py_node/project.json`）。

### pnpm

Node.js 包管理器，用于安装 JavaScript 依赖。在本仓库中主要服务于 nx 和一些 JS 工具脚本。

## 核心开发流程

### ROS 开发

典型的开发循环：

**1) 修改接口** → 接口定义在 `ros/src/interfaces`，修改后需要生成对应的头文件和类型文件，其他包才能引用新的消息类型：

```bash
pixi run ros:build:interfaces
```

**2) 修改 ROS 包** → 构建你修改的包：

```bash
pixi run ros:build <package-name>
```

**3) 代码质量检查** → 包括以下内容：

| 检查项 | 说明 | 命令 |
|--------|------|------|
| format | 代码格式化（Python: ruff format, C++: clang-format） | `pixi run format <pkg>` |
| lint | 静态分析（Python: ruff check, C++: clang-tidy） | `pixi run lint <pkg>` |
| typecheck | 类型检查（Python: ty） | `pixi run typecheck <pkg>` |
| cspell | 拼写检查 | `pixi run cspell <pkg>` |
| test | 单元测试 | `pixi run test <pkg>` |
| copyright | 文件版权头检查 | `pixi run copyright:check` |

`pixi run check <pkg>` 是前五项的组合快捷方式。

**日常开发中提交 commit 时不要求通过所有检查**——这是为了保持开发效率，避免每次小提交都被卡住。但 **PR 前必须通过全部检查**，CI 会强制校验：

```bash
pixi run check:all
pixi run copyright:check
```

### 分支与合并策略

主分支采用 **rebase + squash merge** 的方式维护：开发分支 rebase 到最新 main 后，PR 以 squash and merge 方式合入，保持主分支历史线性且整洁。

## 编码要求

- 代码中**尽量避免中文**。
- 代码中**严禁使用拼音**。

## 文档

### docs/ 目录

`docs/` 目录下存放了详细的技术文档。每个文件顶部包含 [YAML front matter](https://jekyllrb.com/docs/front-matter/)——一段被 `---` 包裹的元数据块，例如：

```markdown
---
description: 质量检查与测试工具链的工具选择、配置位置和执行入口
read_when: 任务涉及 format、lint、typecheck、cspell、test 或文件顶部的 copyright 时
---

# 质量检查与测试工具链
...
```

两个字段的含义：

- **`description`**：这篇文档讲什么，一句话概括。
- **`read_when`**：什么场景下应该阅读这篇文档。

我们基于 front matter 构建了 `pixi run read-docs` 工具（`scripts/read-docs.js`）。不带参数执行时，它会扫描 `docs/` 下所有 `.md` 文件并打印每篇的 `description` 和 `read_when`，方便快速定位需要阅读的文档；带参数执行时会打印指定文档的完整内容：

```bash
pixi run read-docs                        # 列出所有文档的元数据摘要
pixi run read-docs tooling/nx-*.md        # 打印指定文档的完整内容（路径相对于 docs/）
```

这套机制同时服务于人和 AI：人通过摘要快速找到相关文档，AI 工具也通过 `description` 和 `read_when` 判断是否需要加载某篇文档的全文。因此，**写好每篇文档的 front matter 很重要**——描述不准确或不完整会导致 AI 无法找到正确的文档。

你也可以在本地启动文档网站，以更友好的方式浏览 `docs/` 下的全部文档：

```bash
pixi run doc-site
```

启动后访问 `http://localhost:5173` 即可。该站点由 [VitePress](https://vitepress.dev/) 驱动，支持热更新——编辑 Markdown 后页面会自动刷新。

如果你发现文档中有过时或不准确的内容，鼓励你直接修改并提交。

