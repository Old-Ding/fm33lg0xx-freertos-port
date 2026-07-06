# gpio_blink_mdk

这是 FM33LG0xx FreeRTOS 移植的最小验证示例，用于确认 Cortex-M0 / Keil ARMCC5 下 FreeRTOS 能接管 SysTick、创建任务并通过 `vTaskDelay()` 驱动 PB4 LED 周期闪烁。

## 数据流

1. `main()` 完成 IWDT、SVD、BOR、时钟和 GPIO 初始化。
2. `main()` 创建 `LedBlinkTask`，并把任务创建结果记录到 `g_ledTaskCreateStatus`。
3. `LedBlinkTask()` 周期喂狗、执行 SVD 掉电监测、控制 PB4 LED 亮灭。
4. 调度器启动后，任务延时使用 `vTaskDelay()`；SVD 去抖延时通过 `SVD_DelayUs()` 覆盖层避免重配 SysTick。

## 硬件连接

| 功能 | 引脚/外设 | 说明 |
| --- | --- | --- |
| LED0 | `GPIOB PIN4` | `LedBlinkTask` 控制亮灭 |

## 构建

Keil GUI：

1. 打开 `MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`。
3. 执行 Build。

PowerShell：

```powershell
.\scripts\build-keil.ps1 -ExampleName gpio_blink_mdk -CleanAfterBuild
```

## 预期现象

- PB4 LED 约 100 ms 半周期亮灭。
- Keil Watch 中 `g_ledTaskCreateStatus == pdPASS`。
- `g_ledTaskLoopCount` 持续递增。
- `g_ledTaskStackHighWaterMark` 记录 LED task 剩余栈水位。
- `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes` 记录 heap 当前余量和最低水位。
- `g_freertosFaultCode == 0`。
- 如果 `g_freertosFaultCode == 1`，查看 `g_freertosHeapFreeBytes` 和 `g_freertosHeapMinimumEverFreeBytes` 判断 heap 余量。
- 如果 `g_freertosFaultCode == 2`，查看 `g_stackOverflowTaskHandle` 和 `g_stackOverflowTaskName` 定位溢出任务。
- 如果 `g_freertosFaultCode == 6`，查看 `g_freertosAssertFile` 和 `g_freertosAssertLine` 定位 assert 触发位置。

## 关键配置

- `FreeRTOSConfig.h` 将 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler` 映射到 FreeRTOS port。
- Keil 工程引用仓库根目录 `FreeRTOS-Kernel-main`，不在示例内复制内核源码。
- 最小内核源文件集合为 `tasks.c`、`list.c`、`port.c` 和 `heap_4.c`。
- `SVD_DelayUs()` 在应用层覆盖厂商默认延时，原因是调度器运行后 SysTick 归 FreeRTOS 管理。

更完整的板级验收步骤见 [`docs/hardware-validation.md`](../docs/hardware-validation.md)，当前验证级别见 [`docs/validation-status.md`](../docs/validation-status.md)。
