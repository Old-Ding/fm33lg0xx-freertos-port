# 示例说明

本文记录仓库当前维护的 FM33LG0xx FreeRTOS 示例。厂商完整示例目录只作为本地参考，不作为本仓库的开源内容提交。

`examples/examples.json` 是示例构建和文档元数据清单。新增示例时，应把 Keil 工程路径、说明、验证状态和文档入口加入该文件，避免 demo 存在但没有进入统一构建验证或文档维护。

构建后的板级验收步骤见 [硬件验收指南](hardware-validation.md)。

调用链、ISR/task 职责边界和 SysTick 归属见 [架构说明](architecture.md)。

当前示例验证级别见 [验证状态](validation-status.md)。

## 示例清单规则

`examples/examples.json` 使用 `schemaVersion: 1`。每个示例条目只维护这些字段：

- `name`：示例名称，供构建脚本 `-ExampleName` 使用。
- `description`：一句话说明示例目标。
- `project`：Keil `.uvprojx` 工程路径，使用仓库相对路径和正斜杠。
- `target`：Keil target 名称。
- `validationStatus`：`build-verified`、`hardware-verified` 或 `experimental`。
- `documentation`：README、验收文档和验证状态文档入口，使用仓库相对路径和正斜杠。

每个示例必须在 `documentation` 中列出自己的 `<示例目录>/README.md`，公共文档不能替代示例独立说明。

该清单属于项目自有文本文件，必须保持 UTF-8 无 BOM 和 CRLF 换行。

## gpio_blink_mdk

- 目标：验证 FreeRTOS 在 FM33LG02X / Cortex-M0 / Keil ARMCC5 下可以接管 SysTick 并运行任务。
- 核心行为：`LedBlinkTask` 周期喂狗、执行 SVD 掉电监测、翻转 PB4 LED。
- FreeRTOS 源文件：`tasks.c`、`list.c`、`port.c`、`heap_4.c`。
- 观察变量：`g_ledTaskCreateStatus`、`g_ledTaskLoopCount`、`g_ledTaskStackHighWaterMark`、`g_freertosFaultCode`、`g_stackOverflowTaskHandle`、`g_stackOverflowTaskName`、`g_freertosAssertFile`、`g_freertosAssertLine`。

## freertos_signal_adc_uart_mdk

- 目标：在最小移植基线上演示任务间同步和 ISR 唤醒任务。
- 核心行为：PB12 下降沿中断释放信号量，GPIO task 记录事件，ADC task 采样 `FL_ADC_EXTERNAL_CH1` 并通过 UART 输出采样值。
- FreeRTOS 源文件：在最小集合基础上增加 `queue.c`。
- 观察变量：`g_monitorTaskCreateStatus`、`g_gpioTaskCreateStatus`、`g_adcTaskCreateStatus`、`g_monitorTaskLoopCount`、`g_monitorTaskStackHighWaterMark`、`g_gpioTaskStackHighWaterMark`、`g_adcTaskStackHighWaterMark`、`g_gpioIrqCount`、`g_gpioTaskWakeCount`、`g_gpioSemaphoreGiveFailCount`、`g_adcSemaphoreGiveFailCount`、`g_adcSampleMv`、`g_adcSampleCount`、`g_stackOverflowTaskHandle`、`g_stackOverflowTaskName`、`g_freertosAssertFile`、`g_freertosAssertLine`。

## 验证顺序

1. 先确认 `examples/examples.json` 已列出所有需要维护的 Keil 示例。
2. 运行 `.\scripts\build-keil.ps1 -ListExamples`，确认脚本能看到清单中的示例。
3. 开发阶段可用 `.\scripts\build-keil.ps1 -ExampleName freertos_signal_adc_uart_mdk -CleanAfterBuild` 只验证一个示例。
4. 提交前运行 `.\scripts\build-keil.ps1 -CleanAfterBuild`，确认所有示例都是 `0 Error(s), 0 Warning(s)`。
5. 如果只做手工验证，先构建 `gpio_blink_mdk`，确认基础移植未被破坏。
6. 再构建 `examples/freertos_signal_adc_uart_mdk`，确认新增信号量依赖的 `queue.c` 已加入工程。
7. 硬件运行时先看 PB4 LED 是否周期翻转，再触发 PB12 下降沿观察 UART 输出和 ADC 变量。
8. 如果任务未运行，先看任务创建状态和 `g_freertosFaultCode`，再查 `SysTick_Handler`、`PendSV_Handler`、`SVC_Handler` 是否由 FreeRTOS port 接管。
