# FreeRTOS 移植说明

本文记录把 `FreeRTOS-Kernel-main` 移植到 `gpio_blink_mdk` 的步骤，目标是让原来的裸机 GPIO 闪灯在 FreeRTOS 任务中运行。

## 1. 先建模

当前 GPIO 工程的数据流和调用链如下：

1. `Reset_Handler` 调用 `SystemInit()`，初始化芯片基础时钟、调试和低压复位相关寄存器。
2. `main()` 依次初始化 IWDT、厂商延时、SVD、BOR、MF 时钟和 GPIO。
3. 原裸机主循环周期性执行 `IWDT_Clr()`、`PowerDownMonitroing()`、`LED0_ON/OFF()` 和 `FL_DelayMs()`。
4. `FL_Init()` 和 `FL_DelayUs()` 会使用 SysTick，因此调度器启动后不能再用 `FL_DelayMs()` 做周期延时，也不能让 SVD 去抖延时继续重配 SysTick。

移植后的职责划分如下：

1. 启动文件仍只负责异常向量和复位入口，不直接改启动文件。
2. FreeRTOS 的 SVC、PendSV、SysTick 入口在 `FreeRTOSConfig.h` 中映射到启动文件已有弱符号。
3. 应用层只创建一个 `LedBlinkTask`，把原主循环中的喂狗、掉电监测和闪灯节拍迁入任务。
4. 调度器启动前仍允许厂商 `FL_DelayUs()` 用于 SVD 稳定等待；调度器启动后应用周期延时统一使用 `vTaskDelay()`，SVD 微秒级去抖通过 `SVD_DelayUs()` 覆盖实现短忙等。

## 2. 选择 FreeRTOS port

Keil 工程文件中目标芯片为 `FM33LG02X`，CPU 为 `Cortex-M0`，编译器为 ARMCC5，因此选择：

1. 头文件目录：`..\..\FreeRTOS-Kernel-main\include`
2. 移植层目录：`..\..\FreeRTOS-Kernel-main\portable\RVDS\ARM_CM0`
3. 内存管理：`..\..\FreeRTOS-Kernel-main\portable\MemMang\heap_4.c`

这里没有选择 GCC 或 IAR port，因为当前工程是 Keil ARMCC5；也没有改启动文件，因为启动文件中的 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler` 已经是弱符号，配置层映射即可接管。

## 3. 新增 FreeRTOSConfig.h

文件位置：`gpio_blink_mdk\Inc\FreeRTOSConfig.h`

关键配置：

1. `configCPU_CLOCK_HZ` 使用 `SystemCoreClock`，避免 FreeRTOS 和厂商时钟各维护一份频率。
2. `configTICK_RATE_HZ` 设为 `1000U`，`pdMS_TO_TICKS(100U)` 可直接对应 100 ms 闪灯节拍。
3. `configTOTAL_HEAP_SIZE` 设为 `4 KB`，当前只创建 idle task 和一个 LED task，足够运行并保留余量。
4. `configUSE_MALLOC_FAILED_HOOK` 和 `configCHECK_FOR_STACK_OVERFLOW` 打开，方便调试任务创建失败和栈溢出。
5. 中断入口映射如下：

```c
#define vPortSVCHandler      SVC_Handler
#define xPortPendSVHandler   PendSV_Handler
#define xPortSysTickHandler  SysTick_Handler
```

## 4. 更新 Keil 工程

在 `gpio_blink_mdk\MDK-ARM\FM33LG0XX_Tester.uvprojx` 中做两类修改。

第一类是 include path 增加 FreeRTOS 头文件和 Cortex-M0 port：

```text
..\..\FreeRTOS-Kernel-main\include
..\..\FreeRTOS-Kernel-main\portable\RVDS\ARM_CM0
```

第二类是新增 `Middlewares/FreeRTOS` 文件组，加入最小可运行源文件：

```text
..\..\FreeRTOS-Kernel-main\tasks.c
..\..\FreeRTOS-Kernel-main\list.c
..\..\FreeRTOS-Kernel-main\portable\RVDS\ARM_CM0\port.c
..\..\FreeRTOS-Kernel-main\portable\MemMang\heap_4.c
```

当前闪灯任务不使用队列、软件定时器、事件组和流缓冲，所以没有把 `queue.c`、`timers.c`、`event_groups.c`、`stream_buffer.c` 加入工程。后续确实用到对应 API 时，再把对应源文件加入工程并打开配置项。

## 5. 修改 main.c

文件位置：`gpio_blink_mdk\Src\main.c`

移植后的主流程：

1. 保留原来的 IWDT、SVD、BOR、时钟和 GPIO 初始化顺序。
2. 用 `xTaskCreate()` 创建 `LedBlinkTask`。
3. 用 `vTaskStartScheduler()` 启动调度器。
4. 如果任务创建失败或调度器返回，通过 `g_freertosFaultCode` 暴露失败原因。

`LedBlinkTask` 保留原主循环职责：

```c
IWDT_Clr();
PowerDownMonitroing();
LED0_ON();
vTaskDelay(pdMS_TO_TICKS(100U));
IWDT_Clr();
PowerDownMonitroing();
LED0_OFF();
vTaskDelay(pdMS_TO_TICKS(100U));
```

这样没有在 idle hook、tick hook 或其他位置重复喂狗，喂狗职责仍只有 LED 任务一处。

## 6. 处理 SysTick 归属

厂商 `SVD_Result_Confirmed()` 原来直接调用 `FL_DelayUs()`，而 `FL_DelayUs()` 会重写 SysTick 装载值。FreeRTOS 启动后 SysTick 是内核节拍源，所以这里不能在任务里继续直接使用 `FL_DelayUs()`。

本次在 SVD 层只做一个职责明确的改动：

```c
__WEAK void SVD_DelayUs(uint32_t u32Delay_us)
{
    FL_DelayUs(u32Delay_us);
}
```

`SVD_Result_Confirmed()` 仍是唯一的 SVD 结果确认函数，只把内部延时调用从 `FL_DelayUs()` 换成 `SVD_DelayUs()`。GPIO FreeRTOS 工程在 `main.c` 中覆盖 `SVD_DelayUs()`：

1. 调度器未启动时，继续调用 `FL_DelayUs()`，保证启动阶段行为不变。
2. 调度器已启动时，使用不改 SysTick 的短忙等，只服务 SVD 去抖这类微秒级等待。
3. 毫秒级任务节拍仍使用 `vTaskDelay()`，不把业务延时塞进忙等。

## 7. 调试观察点

可在 Keil Watch 窗口观察：

1. `g_ledTaskCreateStatus`：`pdPASS` 表示 LED 任务创建成功。
2. `g_ledTaskLoopCount`：持续增加表示调度器、SysTick 和任务延时都在运行。
3. `g_ledTaskStackHighWaterMark`：记录 LED 任务剩余栈水位，用于在溢出前评估栈余量。
4. `g_freertosFaultCode`：`0` 表示无 FreeRTOS 故障，`1` 表示 malloc 失败，`2` 表示任务栈溢出，`3` 表示调度器启动失败或异常返回，`6` 表示 FreeRTOS assert 失败。
5. `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes`：运行中记录 heap 当前余量和最低水位，`g_freertosFaultCode == 1` 时保留 malloc 失败现场。
6. `g_freertosAssertFile`、`g_freertosAssertLine`：`g_freertosFaultCode == 6` 时记录 assert 触发位置。

如果 LED 不闪，优先按以下顺序查：

1. 确认 `g_ledTaskCreateStatus == pdPASS`。
2. 确认 `g_freertosFaultCode == 0`。
3. 确认 `SysTick_Handler`、`PendSV_Handler` 符号来自 FreeRTOS `port.c`，不是启动文件弱符号。
4. 确认 `SystemCoreClock` 与实际 HCLK 一致，否则 FreeRTOS tick 周期会不准。

## 8. 后续扩展

如果后续要加更多任务，优先只改应用层：

1. 新任务只用 `vTaskDelay()`、队列或通知等待，不再调用 `FL_DelayMs()` 做阻塞周期延时。
2. 如果使用队列或信号量，把 `queue.c` 加入 Keil 工程，并按需打开 mutex/semaphore 配置。
3. 如果使用软件定时器，把 `timers.c` 加入 Keil 工程，并设置 `configUSE_TIMERS`、`configTIMER_TASK_PRIORITY`、`configTIMER_QUEUE_LENGTH`、`configTIMER_TASK_STACK_DEPTH`。
4. 中断中只做实时采样或唤醒任务，业务逻辑放任务状态机里处理。
