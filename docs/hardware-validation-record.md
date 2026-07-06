# 硬件验收记录模板

本文定义板级验收结果的最小记录格式。它用于 GitHub hardware validation issue、release notes，或后续需要把示例从 `build-verified` 升级到 `hardware-verified` 的提交说明。

没有按本文记录关键证据前，不要把 `examples/examples.json` 或 `docs/validation-status.md` 中的状态升级为 `hardware-verified`。

## 使用时机

- `gpio_blink_mdk` 或 `freertos_signal_adc_uart_mdk` 首次在某块板子上完成验收。
- 更换芯片批次、板卡、Keil 版本、DFP 设备包或调试器后，需要重新确认结果。
- 发布版本前，需要在 release notes 中说明硬件已验证或硬件待验证。
- issue 中报告硬件现象异常，需要记录足够信息用于复现。

## 最小证据

每条硬件验收记录至少包含：

- Git commit。
- 芯片/板卡和关键接线。
- Keil MDK、ARMCC、DFP 设备包和调试器版本。
- `scripts/build-keil.ps1` 构建结果。
- LED、GPIO interrupt、ADC、UART 的实际现象。
- Keil Watch 关键变量。
- 结论：通过、未通过，或仅构建通过但硬件待验证。

`hardware-verified` 只适用于对应示例所有必需现象都通过，并且 `g_freertosFaultCode == 0` 的记录。

## 记录模板

复制以下模板到 GitHub issue、release notes 或验证提交说明中。

````markdown
## 硬件验收记录

- 日期：
- 验收人：
- Git commit：
- 分支：
- 示例：
- 结论：通过 / 未通过 / 构建已验证，硬件待验证

## 环境

- 芯片型号和丝印：
- 板卡名称或版本：
- Keil MDK 版本：
- ARMCC 版本：
- DFP 设备包版本：
- 调试器型号和固件版本：
- 串口工具：

## 构建结果

命令：

```powershell
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
```

关键输出：

```text
gpio_blink_mdk: 0 error(s), 0 warning(s)
freertos_signal_adc_uart_mdk: 0 error(s), 0 warning(s)
```

## 接线

| 功能 | 实际连接 | 电平/参数 | 备注 |
| --- | --- | --- | --- |
| LED0 / PB4 | | | |
| GPIO interrupt / PB12 | | 默认高电平，下降沿触发 | |
| ADC input / PD1 | | mV | |
| UART0 / PA2 TX | | 115200 8E1 | |
| GND | | | |

## gpio_blink_mdk 现象

- [ ] PB4 LED 按约 100 ms 半周期亮灭
- [ ] `g_ledTaskCreateStatus == pdPASS`
- [ ] `g_ledTaskLoopCount` 持续递增
- [ ] `g_freertosFaultCode == 0`

Watch 变量：

```text
g_ledTaskCreateStatus =
g_ledTaskLoopCount =
g_freertosFaultCode =
```

## freertos_signal_adc_uart_mdk 现象

- [ ] PB4 monitor LED 每 500 ms 翻转
- [ ] PB12 下降沿触发后 `g_gpioIrqCount` 递增
- [ ] GPIO task 唤醒后 `g_gpioTaskWakeCount` 递增
- [ ] ADC task 采样后 `g_adcSampleCount` 递增
- [ ] `g_adcSampleMv` 与 PD1 输入电压一致或在可解释误差范围内
- [ ] UART 输出启动日志和 ADC 采样值
- [ ] `g_freertosFaultCode == 0`

UART 输出：

```text
FM33LG0xx FreeRTOS signal ADC UART demo start
ADC CH1: <mv> mV, count=<n>
```

Watch 变量：

```text
g_monitorTaskCreateStatus =
g_gpioTaskCreateStatus =
g_adcTaskCreateStatus =
g_monitorTaskLoopCount =
g_gpioIrqCount =
g_gpioTaskWakeCount =
g_adcSampleMv =
g_adcSampleCount =
g_freertosFaultCode =
```

## 异常和处理

- 现象：
- 已排查项：
- 结论：
- 后续动作：
````

## 状态更新规则

验收通过后再做这些更新：

1. 把对应示例的记录保存在 GitHub issue、release notes 或提交说明中。
2. 将 `examples/examples.json` 中对应示例的 `validationStatus` 从 `build-verified` 改为 `hardware-verified`。
3. 同步更新 `docs/validation-status.md`，把构建证据和硬件证据写清楚。
4. 如果硬件现象依赖特定接线、板卡版本或外部上拉，继续在 `docs/known-limitations.md` 中保留限制说明。

如果任一必需现象未通过，只记录问题，不升级验证状态。
