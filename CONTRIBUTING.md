# 贡献说明

## 基本原则

- 先梳理数据流和调用链，再改代码。
- 只在唯一职责层修改，不做双保险式重复判断。
- 能小改就小改，不做无关重构。
- 注释解释为什么这样处理，不重复翻译代码。
- 关键故障状态要能观察，例如 fault code、任务创建结果、循环计数。

## 提交前检查

1. 确认没有提交 Keil/JLink 生成产物。
2. 确认 `FreeRTOSConfig.h` 中的中断入口映射仍然和启动文件弱符号匹配。
3. 确认新增任务没有使用会重配 SysTick 的厂商阻塞延时。
4. 如果新增 FreeRTOS API，确认对应内核源文件已加入 Keil 工程。
5. 如果新增示例，确认 `examples/examples.json` 已加入对应 Keil 工程。
6. 运行 `.\scripts\check-repo.ps1`，确认示例清单、共享 FreeRTOS 引用和禁止跟踪文件都符合仓库规则。
7. 开发阶段可用 `.\scripts\build-keil.ps1 -ExampleName <name> -CleanAfterBuild` 快速验证单个示例。
8. 提交前运行 `.\scripts\build-keil.ps1 -CleanAfterBuild`，确认所有 Keil 示例都是 `0 Error(s), 0 Warning(s)`。
9. 更新 `docs/porting-notes.md` 或 README 中受影响的构建/调试说明。

## Issue 和 PR

- 报 bug 时请使用 bug report 模板，带上芯片/板卡、Keil、DFP、commit、构建输出和 Watch 变量。
- 提交硬件验收结果时请使用 hardware validation 模板，记录接线、UART 输出和关键计数变量。
- 建议新增示例时请使用 example request 模板，先说明要验证的 FreeRTOS 能力、外设组合和验收标准。
- 提 PR 时请按模板说明数据流和职责边界；涉及 ISR 和任务同步时，说明 ISR 只做实时事件处理，业务逻辑在哪个 task 或状态机完成。

## 发布

- 发布前按 `docs/release-process.md` 运行仓库自检、Keil 全量构建、空白检查和工作区检查。
- `CHANGELOG.md` 的 `Unreleased` 只能表示尚未发布的变化；打 tag 前必须移动到具体版本号和日期。
- 硬件未完成验证的内容必须在 release notes 中标注为待验证，不能写成已验证。

## 代码风格

- 保持现有 C 代码缩进和命名风格。
- 中文注释使用 UTF-8 编码。
- 换行使用 CRLF。
- 新增宏和调试变量要集中定义，避免散落魔数。
