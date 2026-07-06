# 架构与职责边界

本文用于维护 FM33LG0xx FreeRTOS 示例库的调用链和职责边界。它说明当前两个示例为什么这样分层，后续新增 demo 时应优先保持这些约束。

## 仓库层次

| 层次 | 职责 | 当前文件 |
| --- | --- | --- |
| FreeRTOS Kernel | 提供调度、任务、同步对象和 Cortex-M0 port。 | `FreeRTOS-Kernel-main/` |
| 厂商 SDK / CMSIS | 提供启动文件、设备头文件、FL 驱动和系统初始化。 | 各示例 `Drivers/`、`MF-config/` |
| 应用初始化 | 初始化 IWDT、SVD、BOR、时钟、GPIO、ADC、UART，再创建任务和同步对象。 | 示例 `Src/main.c` |
| 实时入口 | 处理中断标志，只把事件交给 FreeRTOS 同步对象。 | `GPIO_IRQHandler()`、`GPIO_IRQCallBack()` |
| 业务任务 | 执行周期动作、事件计数、ADC 采样和调试输出。 | `LedBlinkTask`、`MonitorTask`、`GPIOTask`、`AdcTask` |
| 可观测性 | 暴露任务创建状态、循环计数、IRQ 计数、采样值和 fault code。 | 全局 `volatile` Watch 变量 |

## 启动链路

`gpio_blink_mdk` 的最小链路：

```text
Reset_Handler
  -> SystemInit()
  -> main()
     -> IWDT/SVD/BOR/clock/GPIO init
     -> xTaskCreate(LedBlinkTask)
     -> vTaskStartScheduler()
     -> LedBlinkTask: IWDT_Clr + PowerDownMonitroing + LED toggle + vTaskDelay
```

`freertos_signal_adc_uart_mdk` 的综合链路：

```text
Reset_Handler
  -> SystemInit()
  -> main()
     -> IWDT/SVD/BOR/clock/GPIO/ADC/UART init
     -> xSemaphoreCreateBinary(g_gpioSemaphore)
     -> xSemaphoreCreateCounting(g_adcSemaphore)
     -> xTaskCreate(MonitorTask/GPIOTask/AdcTask)
     -> GPIO_InterruptInit()
     -> vTaskStartScheduler()
```

这里把 `GPIO_InterruptInit()` 放在同步对象和任务创建之后，是为了保证 PB12 中断第一次触发时 ISR 已经有合法的信号量句柄可以释放。

## 中断到任务的数据流

`freertos_signal_adc_uart_mdk` 的 PB12 事件链路：

```text
PB12 falling edge
  -> GPIO_IRQHandler()
     -> clear EXTI flag
     -> GPIO_IRQCallBack()
        -> g_gpioIrqCount++
        -> xSemaphoreGiveFromISR(g_gpioSemaphore, &xHigherPriorityTaskWoken)
        -> xSemaphoreGiveFromISR(g_adcSemaphore, &xHigherPriorityTaskWoken)
        -> 信号量已满时递增 give-fail 计数
        -> portYIELD_FROM_ISR(xHigherPriorityTaskWoken)
  -> GPIOTask wakes and updates g_gpioTaskWakeCount
  -> AdcTask wakes, samples FL_ADC_EXTERNAL_CH1, updates g_adcSampleMv and prints UART
```

职责边界：

- ISR 只处理实时事件：清标志、递增 IRQ 计数、释放同步对象、记录信号量已满导致的事件合并、请求必要的任务切换。
- GPIO task 只记录 GPIO 事件被任务层消费的次数。
- ADC task 只在事件驱动下采样和输出调试信息。
- UART printf 是观测手段，不是任务同步或时序依赖。

## SysTick 归属

调度器启动前，厂商 `FL_DelayUs()` 可用于启动阶段等待。调度器启动后，SysTick 归 FreeRTOS port 管理，应用任务不能再用会重配 SysTick 的厂商阻塞延时。

当前唯一的延时覆盖层是 `SVD_DelayUs()`：

- 调度器未启动：转调 `FL_DelayUs()`，保持启动阶段行为。
- 调度器已启动：使用不改 SysTick 的短忙等，只服务 SVD 去抖。
- 任务周期延时：使用 `vTaskDelay()` 或同步对象等待。

不要在业务任务里重复判断 SysTick 归属；如果新增外设也需要微秒级等待，先判断它是否属于外设驱动初始化阶段，或是否应该抽象到对应驱动层。

## FreeRTOS 源文件依赖

当前示例的 FreeRTOS 源文件选择保持最小集合：

| 能力 | 必需源文件 | 当前使用 |
| --- | --- | --- |
| 任务调度 | `tasks.c`、`list.c`、`port.c`、`heap_4.c` | 两个示例 |
| 二值/计数信号量、队列、mutex | `queue.c` | `freertos_signal_adc_uart_mdk` |
| 软件定时器 | `timers.c` 和 timer task 配置 | 暂未使用 |
| Event group | `event_groups.c` | 暂未使用 |
| Stream/message buffer | `stream_buffer.c` | 暂未使用 |

新增 FreeRTOS API 时，先确认 `FreeRTOSConfig.h` 配置，再确认 Keil 工程里加入对应内核源文件。只改配置不加源文件会导致链接或运行能力缺失。

## 可观测性约定

每个示例至少保留这些观测点：

- 任务创建结果，例如 `g_ledTaskCreateStatus`、`g_monitorTaskCreateStatus`、`g_gpioTaskCreateStatus`、`g_adcTaskCreateStatus`。
- 运行计数，例如 `g_ledTaskLoopCount`、`g_monitorTaskLoopCount`、`g_gpioIrqCount`、`g_gpioTaskWakeCount`、`g_adcSampleCount`。
- 栈水位，例如 `g_ledTaskStackHighWaterMark`、`g_monitorTaskStackHighWaterMark`、`g_gpioTaskStackHighWaterMark`、`g_adcTaskStackHighWaterMark`。
- 同步对象压力计数，例如 `g_gpioSemaphoreGiveFailCount`、`g_adcSemaphoreGiveFailCount`，用于观察 ISR 事件被合并或计数信号量已满。
- 最近一次关键数据，例如 `g_adcSampleMv`。
- 集中 fault code，例如 malloc 失败、栈溢出、同步对象创建失败、任务创建失败和调度器异常返回。
- Heap 余量，例如 `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes`，用于运行中观察 heap 当前余量和最低水位。
- 栈溢出现场，例如 `g_stackOverflowTaskHandle`、`g_stackOverflowTaskName`，用于定位触发 hook 的任务。
- Assert 现场，例如 `g_freertosAssertFile`、`g_freertosAssertLine`，用于定位 FreeRTOS 参数或状态错误。

这些变量是硬件验收和 issue 复现信息的一部分，不应因为“只用于调试”而随意删除。

## 新增示例决策

优先扩展现有示例的情况：

- 只是换 LED、UART、ADC 或 GPIO 引脚。
- 只是补充硬件验收记录、README 或调试变量说明。
- 只是调整任务周期或栈大小，且不引入新的 FreeRTOS 能力。

适合新增示例的情况：

- 要验证新的 FreeRTOS 能力，例如软件定时器、event group、stream buffer 或 tickless idle。
- 要组合新的外设数据流，且会让现有 demo 的职责变得混杂。
- 要保留一个可长期复现的硬件场景，和现有示例的验收目标不同。

新增示例前先按 `docs/new-example-checklist.md` 建模，新增后同步更新 `examples/examples.json`、示例 README、`docs/examples.md`、`docs/hardware-validation.md` 和 `docs/validation-status.md`。
