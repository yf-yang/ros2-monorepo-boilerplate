## 时刻更新文档
保持信息的更新至关重要
当架构、工具链等信息发生变化时，你应当考虑更新文档
当你不得不自行检索仓库以获得架构、工具链等信息时，这说明它们没有被很好的记录，你应当考虑更新文档
当你具体探查代码的实现逻辑与细节时，它们可能适合被写到某篇特定文档中，也有可能不适合记录在文档中，你需要仔细甄别
不要静悄悄的更新文档，而是在你的任务完成之后，给出你准备对文档做出的调整，请求人类确认后再变更

## 目录结构
这是一个Monorepo，主要的目录如下：
- ROS2工作区：`ros/`（消息定义在`src/interfaces/`，`src/`下其它目录是不同的 ros 节点）
- `docs/`参见下文`文档`

## 文档
仓库中存在三类文档：
-`AGENTS.md`（被`CLAUDE.md`软链接）：嵌入AI上下文的公共信息
-`README.md`：提供给人类阅读的信息，包含了人类不一定掌握的额外信息的介绍。AI在查阅信息时，应优先查阅`docs/`的信息，最后查阅`README.md`，因为`README.md`的内容可能是冗余的
-`docs/`带有YAML front matter的Markdown文件。每个文件包含`title`、`description`和`read_when`的元数据，以便AI筛选出应阅读的文档。两者的区分标准：
  - `description`：概括文档涵盖了哪些内容（回答"这篇文档讲了什么"）
  - `read_when`：概括什么场景下应阅读该文档（回答"我什么时候需要读它"）。粒度应适中——太细会遗漏场景导致AI未能读取，太粗则失去筛选价值
  - 判断重复的方法：如果把`description`的关键词加上"任务涉及…时"就能得到`read_when`，说明两者本质重复。`read_when`应从用户意图/任务目标出发描述，而非简单重述`description`的关键词
当用户给出的信息不足以完成任务时，你应当优先查询文档，再检索代码库

### 查阅文档
```
pixi run list-docs [pattern...]  # 列出匹配的文档元数据（不提供pattern则列出全部）
pixi run read-docs <pattern...>  # 打印匹配文档的完整内容（token消耗大，应先用list-docs确认目标再使用）
```
pattern 为 glob，相对于 `docs/` 目录匹配文件路径。多个 pattern 之间是 OR 关系。
### 更新文档
文档里可以使用中文
- 更新`AGENTS.md`：只有各类AI请求需要阅读的公共信息才应更新到此文件中，例如规范、常用开发工作流、仓库的通用架构等。你应当用尽量简练的语言描述必要信息，由于AI的知识储备很丰富，因此不需要解释太多。细节信息应当调整到`docs/`下的文档中
- 更新`docs/**/*.md`：每个文档应仔细考虑其description和read_when的设置，AI查阅文档时会优先读取这些信息，其披露的准确和完整决定了AI会不会阅读该篇文档的全文。其中`description`概括文档涵盖了哪些内容，`read_when`描述在什么任务场景下应阅读该文档，两者侧重不同，避免用词重复。你应当用尽量简练的语言描述必要信息，由于AI的知识储备很丰富，因此不需要解释太多
- 更新`README.md`：这是给人类看的文档，措辞应当友好详尽但不应出现低信息量的废话。其中不但要包含怎么做，还要包含“相关工具是什么”的讲解和“为什么要这么做”的解释，并在适合举例的地方举例子以便于人类理解

## 编码规范
- 代码里尽量不要出现中文
- 代码里严禁使用拼音

## 工具链与环境
- 本仓库应位于devcontainer中开发
- `pixi`用于所有脚本/命令的执行，并管理c++/python依赖。打开vscode integrated terminal的时候，`pixi`会自动激活虚拟环境，并加载`.pixi/envs/default/bin/python`作为默认解释器
- `nx`用于任务编排和提供缓存，与主要工作流相关的任务都由`pixi`调用`nx`执行

## 开发工作流
- 构建ROS包
```
pixi run ros:build:interfaces # 更新ros/src/interfaces后，生成必要的头文件/类型文件
pixi run ros:build <package-name> # 构建单个ROS包
pixi run ros:build:all
```
- 代码质量
注：单次开发/commit不要求通过所有检查，本仓库仅在CI门禁中对其严格要求
你应当在执行任务完成后做代码质量的检查并修复问题
对于比较大的架构/feature改动，你应优先确保功能的完整与正确，其后才是代码质量。单次任务中代码质量检查未通过也是允许的，你可以将其留到整体实现完成后再检查修复，或者在量很大时，告知人类再另行修复
```
pixi run test <package-name>
pixi run format <package-name>
pixi run lint <package-name>
pixi run typecheck <package-name>
pixi run cspell <package-name>
pixi run check <package-name> # 执行 test、format、lint、typecheck、cspell

pixi run test:all
pixi run format:all
pixi run lint:all
pixi run typecheck:all
pixi run cspell:all
pixi run check:all

pixi run copyright:check # 检查所有文件的版权信息是否完整
pixi run copyright # 添加或更新所有文件的版权信息
```

## 其它
注意环境中安装了zsh的common-aliases插件，当你执行`rm`,`cp`,`mv`等命令时，请使用`\rm``\cp`,`\mv`以规避交互式确认项