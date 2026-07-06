# freertos_signal_adc_uart_mdk

这是基于 `gpio_blink_mdk` 扩展出来的 FreeRTOS 综合基础 demo，用于验证 FM33LG0xx 上的任务调度、信号量、GPIO ISR 唤醒、ADC 采样和 UART 调试输出。

## 数据流

1. `main()` 完成 IWDT、SVD、BOR、时钟、GPIO、ADC、UART 初始化。
2. `main()` 创建 `g_gpioSemaphore` 和 `g_adcSemaphore`，再创建 monitor、GPIO、ADC 三个任务。
3. `GPIO_InterruptInit()` 最后打开 PB12 外部中断，确保 ISR 触发时同步对象已经存在。
4. `GPIO_IRQHandler()` 只清 EXTI 标志并调用 `GPIO_IRQCallBack()`。
5. `GPIO_IRQCallBack()` 从 ISR 中释放 GPIO 二值信号量和 ADC 计数信号量，并通过 `portYIELD_FROM_ISR()` 请求必要的任务切换。
6. `GPIOTask()` 记录 GPIO 触发计数，`AdcTask()` 采样 `FL_ADC_EXTERNAL_CH1` 并通过 UART 输出毫伏值。

## 硬件连接

| 功能 | 引脚/外设 | 说明 |
| --- | --- | --- |
| LED0 monitor | `GPIOB PIN4` | monitor task 每 500 ms 翻转一次 |
| GPIO interrupt | `GPIOB PIN12` / `EXTI LINE7` | 下降沿触发 |
| ADC input | `PD1` / `FL_ADC_EXTERNAL_CH1` | GPIO 事件触发后采样 |
| UART debug | `UART0`，`PA2/PA3` | 115200，8E1，TX only |

## 构建

Keil GUI：

1. 打开 `MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`。
3. 执行 Build。

PowerShell：

```powershell
& '<Keil install path>\UV4\UV4.exe' -b '.\examples\freertos_signal_adc_uart_mdk\MDK-ARM\FM33LG0XX_Tester.uvprojx' -t 'Example'
```

## 预期现象

- PB4 LED 周期翻转。
- PB12 下降沿触发后，GPIO task 的触发计数递增。
- 每次 GPIO 触发也会唤醒 ADC task，UART 输出 ADC CH1 毫伏值。
- Keil Watch 中 `g_monitorTaskLoopCount`、`g_gpioIrqCount`、`g_gpioTaskWakeCount`、`g_adcSampleCount` 会随运行递增。
- `g_monitorTaskStackHighWaterMark`、`g_gpioTaskStackHighWaterMark`、`g_adcTaskStackHighWaterMark` 记录各任务剩余栈水位。
- `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes` 记录 heap 当前余量和最低水位。
- 如果 PB12 触发过快，`g_gpioSemaphoreGiveFailCount` 或 `g_adcSemaphoreGiveFailCount` 递增表示对应信号量已满，事件被合并或丢弃。
- 如果 `g_freertosFaultCode == 1`，查看 `g_freertosHeapFreeBytes` 和 `g_freertosHeapMinimumEverFreeBytes` 判断 heap 余量。
- 如果 `g_freertosFaultCode == 2`，查看 `g_stackOverflowTaskHandle` 和 `g_stackOverflowTaskName` 定位溢出任务。
- 如果 `g_freertosFaultCode == 6`，查看 `g_freertosAssertFile` 和 `g_freertosAssertLine` 定位 assert 触发位置。

更完整的接线、串口参数、Watch 变量和故障定位步骤见 [`docs/hardware-validation.md`](../../docs/hardware-validation.md)。

## 关键配置

- `FreeRTOSConfig.h` 启用 `configUSE_COUNTING_SEMAPHORES`。
- Keil 工程加入 `queue.c`，因为二值信号量和计数信号量都依赖 FreeRTOS queue 实现。
- 仍然引用仓库根目录 `FreeRTOS-Kernel-main`，不在 demo 内复制 FreeRTOS 内核。
- SVD 微秒延时仍由 `SVD_DelayUs()` 统一处理，调度器启动后不再让业务代码调用会重配 SysTick 的厂商 delay。
