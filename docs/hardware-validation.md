# 硬件验收指南

本文用于把 Keil 构建通过后的板级验证步骤固定下来。验收顺序按真实运行链路展开：先确认工程可构建，再确认调度器运行，最后确认 GPIO ISR 通过 FreeRTOS 信号量唤醒 ADC task 并输出 UART 日志。

验收结果按 [硬件验收记录模板](hardware-validation-record.md) 记录。没有记录 Git commit、构建结果、接线、UART 输出和 Watch 变量前，不要把示例状态升级为 `hardware-verified`。

## 前置条件

- Windows 11。
- Keil MDK 5.x，ARMCC5 可用。
- 已安装 `FMSH.FM33LG0XX_DFP.3.0.1` 设备包。
- `UV4.exe` 可用；本机已验证路径为 `D:\keil\MDK542a\UV4\UV4.exe`。
- 调试器可连接目标板，并能通过 Keil 下载程序。
- 串口工具支持 115200，8E1。
- `PB12` 中断输入需要稳定默认高电平；当前 demo 没有打开内部上下拉，建议用板载电路或外部上拉后再拉低触发下降沿。
- `PD1` ADC 输入电压应在目标板允许范围内，避免超过 `VDDA`。

## 构建验收

提交前建议运行全部示例构建：

```powershell
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
```

开发单个综合 demo 时可以只构建该示例：

```powershell
.\scripts\build-keil.ps1 -ExampleName freertos_signal_adc_uart_mdk -CleanAfterBuild
```

通过标准：

- `gpio_blink_mdk` 为 `0 Error(s), 0 Warning(s)`。
- `freertos_signal_adc_uart_mdk` 为 `0 Error(s), 0 Warning(s)`。
- 构建结束后 `MDK-ARM/Objects`、`MDK-ARM/Listings` 等生成物被清理，不进入 Git。

## gpio_blink_mdk 验收

硬件连接：

| 功能 | 引脚/外设 | 预期 |
| --- | --- | --- |
| LED0 | `GPIOB PIN4` | LED 周期闪烁 |

运行步骤：

1. 用 Keil 打开 `gpio_blink_mdk/MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`，编译并下载。
3. 复位目标板，观察 `PB4` LED。
4. 在 Keil Watch 中加入关键变量。

通过标准：

- `PB4` LED 按约 100 ms 半周期亮灭。
- `g_ledTaskCreateStatus == pdPASS`。
- `g_ledTaskLoopCount` 持续递增。
- `g_freertosFaultCode == 0`。

## freertos_signal_adc_uart_mdk 验收

硬件连接：

| 功能 | 引脚/外设 | 预期 |
| --- | --- | --- |
| LED0 monitor | `GPIOB PIN4` | monitor task 每 500 ms 翻转一次 |
| GPIO interrupt | `GPIOB PIN12` / `EXTI LINE7` | 默认高电平，下降沿触发 |
| ADC input | `PD1` / `FL_ADC_EXTERNAL_CH1` | PB12 触发后采样 |
| UART debug | `UART0`，`PA2/PA3` | 115200，8E1，TX only |

运行步骤：

1. 用 Keil 打开 `examples/freertos_signal_adc_uart_mdk/MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`，编译并下载。
3. 打开串口工具，参数设为 115200，8 数据位，偶校验，1 停止位。
4. 复位目标板，先确认 `PB4` LED 周期翻转。
5. 将 `PB12` 从稳定高电平拉低，触发一次下降沿。
6. 观察串口输出和 Keil Watch 变量。

预期串口输出：

```text
FM33LG0xx FreeRTOS signal ADC UART demo start
ADC CH1: <mv> mV, count=<n>
```

Watch 变量：

| 变量 | 通过标准 |
| --- | --- |
| `g_monitorTaskCreateStatus` | `pdPASS` |
| `g_gpioTaskCreateStatus` | `pdPASS` |
| `g_adcTaskCreateStatus` | `pdPASS` |
| `g_monitorTaskLoopCount` | 持续递增 |
| `g_gpioIrqCount` | 每次 PB12 下降沿后递增 |
| `g_gpioTaskWakeCount` | GPIO task 被信号量唤醒后递增 |
| `g_adcSampleMv` | 反映 `PD1` 当前输入电压 |
| `g_adcSampleCount` | ADC task 每次采样后递增 |
| `g_freertosFaultCode` | 正常为 `0` |

## 故障定位

| 症状 | 优先检查 |
| --- | --- |
| Keil 构建失败 | `UV4.exe` 路径、DFP 设备包、target 是否为 `Example` |
| LED 不闪 | 任务创建状态、`g_freertosFaultCode`、`SysTick_Handler` / `PendSV_Handler` / `SVC_Handler` 是否由 FreeRTOS port 接管 |
| `g_freertosFaultCode == 1` | FreeRTOS heap 不足，检查 `configTOTAL_HEAP_SIZE` 和任务栈大小 |
| `g_freertosFaultCode == 2` | 任务栈溢出，先看最近新增任务的 stack words |
| `g_freertosFaultCode == 3` | 调度器启动失败或异常返回，检查 FreeRTOS port 和中断向量映射 |
| `g_freertosFaultCode == 4` | 信号量创建失败，检查 `queue.c` 是否加入工程以及 heap 是否足够 |
| `g_freertosFaultCode == 5` | 任务创建失败，检查任务栈和任务数量 |
| PB12 无触发 | PB12 是否有稳定默认高电平、是否形成下降沿、`EXTI LINE7` 是否映射到 `PB12` |
| ADC 数值不变 | `PD1` 输入电压、`FL_ADC_EXTERNAL_CH1`、目标板 `VDDA` 和地线连接 |
| 串口无输出 | `PA2` TX 接线、串口参数 115200 8E1、串口工具是否选择正确端口 |

## 发布前检查

1. 运行 `.\scripts\build-keil.ps1 -CleanAfterBuild`。
2. 运行 `git diff --check`。
3. 运行 `git status --short --ignored`，确认只有 `/例程/` 等本地参考目录处于 ignored 状态。
4. 确认没有提交 `Objects/`、`Listings/`、`.uvoptx`、`.uvguix.*`、`.axf`、`.hex`、`.map`、`.crf`、`.o`、`.d` 等生成物。
5. 如果要声明硬件已验证，按 [硬件验收记录模板](hardware-validation-record.md) 补齐可追溯证据。
