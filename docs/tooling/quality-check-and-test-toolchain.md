---
title: 质量检查与测试工具链
description: 质量检查与测试工具链的工具选择、配置位置和执行入口（包级与文件级）
read_when: 需要执行或配置 format、lint、typecheck、cspell、test 或 copyright 检查时
---

# 质量检查与测试工具链

每个工具提供两层入口：

- **包级**（经 nx 编排，带缓存）：`pixi run format <pkg>` 等
- **文件级**（直接调用工具，不经 nx）：`pixi run clang-format <file...>` 等

文件级命名约定：默认 = 只检查不修改；`:fix` = 写入/修复。

## format

Python：

- 工具：`ruff format`
- 配置：`ruff.toml`
- 包级入口：`pixi run format <package-name>` / `pixi run format:all`
- 文件级入口：`pixi run ruff-format <file...>`（检查） / `pixi run ruff-format:fix <file...>`（写入）
- 文档：https://docs.astral.sh/ruff/formatter/
- 可配置选项：https://docs.astral.sh/ruff/settings/

C++：

<!-- cspell:disable-next-line -->
- 工具：`clang-format`
- 配置：`ros/.clang-format`
- 包级入口：`pixi run format <package-name>`（例如 `demo_cpp_node`）
- 文件级入口：`pixi run clang-format <file...>`（检查） / `pixi run clang-format:fix <file...>`（写入）
- 文档：https://clang.llvm.org/docs/ClangFormat.html
- 规则：https://clang.llvm.org/docs/ClangFormatStyleOptions.html

TypeScript：

- 工具：`oxfmt`
- 配置：`.oxfmtrc.json`
- 包级入口：`pixi run format <package-name>` / `pixi run format:all`
- 文件级入口：`pixi run oxfmt <file...>`（检查） / `pixi run oxfmt:fix <file...>`（写入）
- 文档：https://oxc.rs/docs/guide/usage/formatter
- 可配置选项：https://oxc.rs/docs/guide/usage/formatter/config-file-reference

## lint

Python：

- 工具：`ruff check`
- 配置：`ruff.toml`
- 包级入口：`pixi run lint <package-name>` / `pixi run lint:all`
- 文件级入口：`pixi run ruff-check <file...>`（检查） / `pixi run ruff-check:fix <file...>`（自动修复）
- 文档：https://docs.astral.sh/ruff/linter/
- 规则：https://docs.astral.sh/ruff/rules/
- 可配置选项：https://docs.astral.sh/ruff/settings/

C++：

- 工具：`clang-tidy`（包级经 `run-clang-tidy` 并行执行）
- 配置：`ros/.clang-tidy`
- 包级入口：`pixi run lint <package-name>`（例如 `demo_cpp_node`、`bridge`）/ `pixi run lint:all`
- 文件级入口：`pixi run clang-tidy <file...>`（首次运行需 configure 生成 `compile_commands.json`，之后复用缓存）
- 前置依赖：`ros_ws:configure`（构建 `interfaces` + CMake configure 其余包，无需完整编译）
- 包级过滤器：`scripts/run-clang-tidy-filter.py`——包装 `run-clang-tidy`，过滤系统/第三方头文件中的编译错误（Clang 21 + GCC 14 libstdc++ 存在不兼容，例如 `<optional>` 与 zenoh 的交互），仅项目源码中的诊断会导致非零退出码
- 文档：https://clang.llvm.org/extra/clang-tidy/
- 规则：https://clang.llvm.org/extra/clang-tidy/checks/list.html

TypeScript：

- 工具：`oxlint --type-aware`
- 配置：`.oxlintrc.json`
- 依赖：`oxlint-tsgolint`（提供 type-aware linting 所需的 tsgo 绑定）
- 包级入口：`pixi run lint <package-name>` / `pixi run lint:all`
- 文件级入口：`pixi run oxlint <file...>`
- 文档：https://oxc.rs/docs/guide/usage/linter
- type-aware 文档：https://oxc.rs/docs/guide/usage/linter/type-aware
- 规则：https://oxc.rs/docs/guide/usage/linter/rules
- 可配置选项：https://oxc.rs/docs/guide/usage/linter/config-file-reference

## typecheck

Python：

- 工具：`ty check`
- 配置：`ty.toml`
- 包级入口：`pixi run typecheck <package-name>` / `pixi run typecheck:all`
- 文件级入口：`pixi run ty <file-or-dir...>`
- 文档：https://docs.astral.sh/ty/
- 规则：https://docs.astral.sh/ty/reference/rules/
- 可配置选项：https://docs.astral.sh/ty/configuration/

<!-- TODO: 当 TypeScript 7 正式发布后，将 @typescript/native-preview 迁移为正式包，并删除此注释 -->

TypeScript：

- 工具：`tsgo --noEmit`（`@typescript/native-preview`，即 TypeScript 7 native 编译器的预览版）
- 配置：`tsconfig.base.json`（根级共享）、各包 `tsconfig.json`（通过 `extends` 继承）
- 包级入口：`pixi run typecheck <package-name>` / `pixi run typecheck:all`
- 备注：`tsgo` 不支持对单个文件进行有效的类型检查（脱离 tsconfig 后会丢失编译选项导致误报），因此没有文件级入口
- 文档：https://www.typescriptlang.org/tsconfig/

## cspell

- 工具：`cspell`
- 配置：`.cspell.json`
- 包级入口：`pixi run cspell <package-name>` / `pixi run cspell:all`
- 文件级入口：`pixi run cspell:files <file...>`
- 备注：包级通过各 package 的 nx target 执行，遵循 `.cspell.json` 中 ignore 与词典配置
- 文档：https://cspell.org/docs/Configuration

## test

统一入口：

- `pixi run test <package-name>`
- `pixi run test:all`

C++：

  - 由 nx target 调用 `colcon test` 与 `colcon test-result --verbose`
  - 使用 `ament_cmake_gtest`
  - 文档（ROS）：https://docs.ros.org/en/humble/Tutorials/Intermediate/Testing/Cpp.html
  - 文档（gtest）：https://google.github.io/googletest/

Python：
  - 通过 ROS/colcon 测试流程执行 `test/` 下用例
  - 文档（ROS）：https://docs.ros.org/en/humble/Tutorials/Intermediate/Testing/Python.html
  - 文档（pytest）：https://docs.pytest.org/en/stable/

## copyright

- 工具：`copywrite`
- 配置：`.copywrite.hcl`
- 命令：
  - 检查预览：`pixi run copyright:check`
  - 执行更新：`pixi run copyright`
- 文档：https://github.com/hashicorp/copywrite/raw/refs/heads/main/README.md
