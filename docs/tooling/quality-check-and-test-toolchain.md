---
title: 质量检查与测试工具链
description: 质量检查与测试工具链的工具选择、配置位置和执行入口
read_when: 需要执行或配置 format、lint、typecheck、cspell、test 或 copyright 检查时
---

# 质量检查与测试工具链

## format

Python：

- 工具：`ruff format --check`
- 配置：`ruff.toml`
- 入口：`pixi run format <package-name>` / `pixi run format:all`
- 文档：https://docs.astral.sh/ruff/formatter/
- 可配置选项：https://docs.astral.sh/ruff/settings/

C++：

<!-- cspell:disable-next-line -->
- 工具：`clang-format --dry-run --Werror`
- 配置：`ros/.clang-format`
- 入口：`pixi run format <package-name>`（例如 `demo_cpp_node`）
- 文档：https://clang.llvm.org/docs/ClangFormat.html
- 规则：https://clang.llvm.org/docs/ClangFormatStyleOptions.html

## lint

Python：

- 工具：`ruff check`
- 配置：`ruff.toml`
- 入口：`pixi run lint <package-name>` / `pixi run lint:all`
- 文档：https://docs.astral.sh/ruff/linter/
- 规则：https://docs.astral.sh/ruff/rules/
- 可配置选项：https://docs.astral.sh/ruff/settings/

C++：

- 工具：`clang-tidy`
- 配置：`ros/.clang-tidy`
- 说明：会基于构建产物生成/汇总 `compile_commands.json` 后再分析
- 入口：`pixi run lint ros_ws`（或直接使用对应 nx target）
- 文档：https://clang.llvm.org/extra/clang-tidy/
- 规则：https://clang.llvm.org/extra/clang-tidy/checks/list.html

## typecheck

Python：

- 工具：`ty check`
- 配置：`ty.toml`
- 入口：`pixi run typecheck <package-name>` / `pixi run typecheck:all`
- 文档：https://docs.astral.sh/ty/
- 规则：https://docs.astral.sh/ty/reference/rules/
- 可配置选项：https://docs.astral.sh/ty/configuration/

## cspell

- 工具：`cspell`
- 配置：`.cspell.json`
- 入口：`pixi run cspell <package-name>` / `pixi run cspell:all`
- 备注：通过各 package 的 nx target 执行，遵循 `.cspell.json` 中 ignore 与词典配置
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