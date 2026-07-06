# Changelog

## Unreleased

- 扩展示例清单元数据，并让仓库自检覆盖示例说明、验证状态和文档入口。
- 完善第三方来源说明，并让仓库自检覆盖关键许可证文件和来源文件头。
- 新增 `docs/release-process.md`，固定版本号、tag、发布前门禁和硬件验收记录规则。
- 新增 GitHub issue 和 PR 模板，规范 bug 复现、硬件验收、新示例请求和贡献自检信息。
- 新增 GitHub Actions 仓库自检，验证示例清单、共享 FreeRTOS 引用和禁止提交的生成物。
- 新增 `docs/hardware-validation.md`，补充板级接线、运行验收、Watch 变量和故障定位步骤。
- `scripts/build-keil.ps1` 支持 `-ListExamples` 和 `-ExampleName`，便于列出或单独构建示例。
- 新增 `examples/examples.json`，让 Keil 构建脚本从示例清单读取工程列表。
- 新增 `scripts/build-keil.ps1`，用于批量 rebuild Keil 示例、解析构建日志并可选清理生成物。
- 新增 `examples/freertos_signal_adc_uart_mdk`，演示 FreeRTOS 任务、信号量、GPIO ISR、ADC 和 UART。
- 更新 README 和示例说明，明确仓库定位为 FM33LG0xx FreeRTOS port and examples。
- 清理 Keil/JLink 生成产物和本机状态文件。
- 将原中文示例目录整理为 `gpio_blink_mdk`。
- 新增 README、贡献说明、变更日志、第三方来源说明和 Git 忽略规则。
- 新增 LICENSE，明确原创部分使用 MIT，第三方文件保留原始许可证。

## 0.1.0 - 2026-06-12

- 在 FM33LG02X GPIO 闪灯工程中加入 FreeRTOS。
- 使用 RVDS `ARM_CM0` port，加入 `tasks.c`、`list.c`、`port.c`、`heap_4.c`。
- 通过 `FreeRTOSConfig.h` 将 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler` 映射到 FreeRTOS port。
- 将 LED 闪灯、喂狗和掉电监测迁移到 `LedBlinkTask`。
- 增加 `SVD_DelayUs()` 弱函数覆盖点，避免调度器启动后 SVD 去抖重配 SysTick。
