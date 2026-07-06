# 已知限制

本文记录当前仓库的能力边界。它用于防止把“已能构建的 demo”误理解成完整产品级 BSP 或全芯片系列支持包。

## 支持范围

- 当前目标芯片按 `FM33LG02X` 验证。
- 当前工具链为 Keil MDK / ARMCC5。
- FreeRTOS port 使用 `FreeRTOS-Kernel-main/portable/RVDS/ARM_CM0`。
- 当前维护示例见 `examples/examples.json`。

## 当前限制

| 类别 | 限制 |
| --- | --- |
| 硬件验证 | 当前示例状态为 `build-verified`，尚未在仓库中记录完整板级验收结果。 |
| CI 构建 | GitHub Actions 只运行不依赖 Keil 的仓库自检；Keil/ARMCC5 构建仍需要本机运行。 |
| 芯片范围 | 未声明覆盖 FM33LG0xx 全系列；移植点以 `FM33LG02X` 工程为基线。 |
| 工具链 | 未提供 GCC、IAR、CMake 或 Makefile 构建入口。 |
| 低功耗 | 暂未实现 tickless idle、deep sleep 或低功耗唤醒 demo。 |
| 运行统计 | 暂未启用 CPU utilization、run-time stats 或 trace recorder。 |
| UART | `freertos_signal_adc_uart_mdk` 当前 UART 用作 printf 调试输出，配置为 TX only。 |
| GPIO 中断 | PB12 输入未打开内部上下拉，硬件验收时需要外部或板载电路提供稳定默认电平。 |
| ADC | ADC demo 使用轮询采样，不覆盖 DMA、连续扫描或多通道采样。 |
| 厂商断言 | Keil 工程启用 `USE_FULL_ASSERT` 时，FL Driver 的 `assert_param` 会停在厂商宏中；这不属于 FreeRTOS `configASSERT`，不会写入 `g_freertosFaultCode` 或 `g_freertosAssertFile`。 |
| HardFault | 当前 `HardFault_Handler` 直接触发 RMU 软复位，不保留 fault stack frame 或 FreeRTOS Watch 现场。 |
| 厂商示例 | 本仓库不提交本地完整 `例程/` 厂商示例集合，只抽取当前 demo 必需文件。 |

## 使用建议

- 把 `gpio_blink_mdk` 作为 FreeRTOS 移植最小基线。
- 新增外设 demo 时，优先基于现有示例扩展，并同步更新 `examples/examples.json`、`docs/examples.md`、`docs/validation-status.md` 和对应 README。
- 发布前按 [发布流程](release-process.md) 运行门禁；硬件未验证的内容必须继续标注为待验证。
- 如果需要声明 `hardware-verified`，先按 [硬件验收指南](hardware-validation.md) 记录板级现象和 Watch 变量。
