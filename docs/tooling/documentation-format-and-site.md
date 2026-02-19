---
title: 文档格式与文档站点
description: docs/ 下 Markdown 文件的 front matter 格式规范、read-docs 脚本的用法与输出，以及 VitePress 文档站点的侧边栏生成与本地预览
read_when: 需要新增或修改 docs/ 下的文档，或需要预览、配置文档站点时
---

# 文档格式与文档站点

## Markdown 文件格式

每个 `docs/` 下的 `.md` 文件必须包含 YAML front matter，字段如下：

```yaml
---
title: 文档标题          # 显示在侧边栏和页面顶部
description: 一句话摘要   # 概括文档涵盖了哪些内容
read_when: 触发条件描述   # 描述在什么任务场景下应阅读该文档
---
```

- `title`：同时作为 VitePress 侧边栏条目名称；缺省时会从文件名生成 Title Case 标题。
- `description`：概括文档**讲了什么**，侧重内容范围。例如："质量检查与测试工具链的工具选择、配置位置和执行入口"。
- `read_when`：概括**什么时候需要读它**，从用户意图或任务目标出发描述。粒度应适中——太细会遗漏场景导致 AI 未能读取，太粗则失去筛选价值。例如："需要执行或配置 format、lint、typecheck、cspell、test 或 copyright 检查时"。

两者不应重复。判断方法：如果把 `description` 的关键词加上"任务涉及…时"就能得到 `read_when`，说明本质重复。`read_when` 应从任务目标出发，而非简单重述 `description` 的关键词。

正文使用标准 Markdown，可以使用中文。正文第一个 `# 标题` 通常与 `title` 保持一致。

### 首页

`docs/index.md` 是站点首页，使用 VitePress 的 `layout: home` 和 `hero` 配置，不需要 `description` / `read_when`。

## read-docs 脚本

`pixi run read-docs` 是面向 AI 和终端用户的文档查阅工具，提供两种用法：

### 列出所有文档的元数据

```bash
pixi run read-docs
```

不传参数时，脚本递归扫描 `docs/` 下所有 `.md` 文件，输出每个文件的路径（相对于 `docs/`）、`title`、`description` 和 `read_when`。示例输出：

```
tooling/quality-check-and-test-toolchain.md - 质量检查与测试工具链
	Description: 质量检查与测试工具链的工具选择、配置位置和执行入口
	Read When: 任务涉及 format、lint、typecheck、cspell、test 或文件顶部的 copyright 时
```

AI 可据此判断哪些文档值得继续阅读全文。

### 打印指定文档的完整内容

```bash
pixi run read-docs <file1> <file2> ...
```

传入一个或多个路径（相对于 `docs/`），脚本输出对应文件的完整正文内容（含 front matter）。例如：

```bash
pixi run read-docs tooling/quality-check-and-test-toolchain.md
```

## VitePress 文档站点

### 启动本地预览

```bash
pixi run doc-site
```

启动后访问终端输出的 URL（默认 `http://localhost:5173`）即可实时预览。

### 侧边栏生成

侧边栏由 `docs/.vitepress/config.ts` 在启动时自动生成：

- 扫描 `docs/` 下所有一级子目录（忽略 `.` 开头的目录）。
- 目录名经 Title Case 转换后作为分组标题。
- 每个目录内的 `.md` 文件按字母序排列，`title` front matter 作为条目名称。

因此新增文档只需将 `.md` 文件放入对应子目录，无需手动修改配置。

### 技术栈

- VitePress（`package.json` 中的 `devDependencies`）
- 依赖通过 `pnpm` 管理，首次使用需执行 `pnpm i`
