# 新增示例 Checklist

本文用于新增 FreeRTOS 示例前后的自检。目标是把“先建模，再动手”的要求落到可执行步骤，避免 demo 能编译但职责边界、文档、验证和来源说明不完整。

## 1. 建模

- 明确示例目标：要验证的 FreeRTOS 能力、外设组合和预期硬件现象。
- 先阅读 `docs/architecture.md`，确认新增内容应该扩展示例还是新增示例。
- 画清数据流：初始化入口、ISR、同步对象、task、UART/Watch 观测点。
- 确认唯一职责层：实时事件放 ISR，业务逻辑放 task 或状态机。
- 确认是否真的需要新增示例；如果只是在现有 demo 上换引脚或参数，优先扩展现有 README 和配置说明。

## 2. 抽取来源

- 可以参考本地 `例程/` 厂商示例，但不要把完整 `例程/` 目录提交到仓库。
- 只抽取当前 demo 必需的厂商 SDK、外设驱动、启动文件或配置文件。
- 保留第三方文件头、版权声明、免责声明和 SPDX 标识。
- 如果新增或替换第三方来源，同步更新 `THIRD_PARTY_NOTICES.md`。

## 3. 建工程

- 默认基于当前已工作的 `gpio_blink_mdk` 或已有 FreeRTOS 示例扩展，不从厂商 FreeRTOS 工程整包复制。
- Keil 工程必须继续通过相对路径引用根目录 `FreeRTOS-Kernel-main`。
- `FreeRTOSConfig.h` 必须把 `vPortSVCHandler`、`xPortPendSVHandler`、`xPortSysTickHandler` 映射到启动文件中的 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler`。
- 新增 FreeRTOS API 时，同步处理 `FreeRTOSConfig.h` 和 Keil 工程源文件：
  - 信号量、队列、mutex 需要 `queue.c`。
  - 软件定时器需要 `timers.c` 和 timer task 配置。
  - event group 需要 `event_groups.c`。
  - stream/message buffer 需要 `stream_buffer.c`。
  - `uxTaskGetStackHighWaterMark()` 需要 `INCLUDE_uxTaskGetStackHighWaterMark = 1`。
- 保留 `configASSERT`，宏使用 `do { ... } while( 0 )` 包装，并通过 `g_freertosAssertFile` 和 `g_freertosAssertLine` 记录断言失败位置，停机前先关中断，避免 FreeRTOS 参数错误变成静默停机。
- 保持 `configUSE_MALLOC_FAILED_HOOK = 1` 和 `configCHECK_FOR_STACK_OVERFLOW = 2`，并在示例自有源码中实现对应 hook。
- 周期监控任务和 malloc failed hook 要记录 `g_freertosHeapFreeBytes` 和 `g_freertosHeapMinimumEverFreeBytes`，便于运行中观察 heap 余量并保留失败现场。
- ISR 中使用 FreeRTOS FromISR API 时，使用局部 `BaseType_t xHigherPriorityTaskWoken = pdFALSE`，并在末尾调用 `portYIELD_FROM_ISR(xHigherPriorityTaskWoken)`。
- 不在 task 中调用会重配 SysTick 的厂商阻塞延时；任务节拍使用 `vTaskDelay()` 或同步对象等待。

## 4. 可观测性

- 每个任务创建结果要能通过 Watch 变量观察。
- 每个任务建议保留 stack high-water mark Watch 变量，用于在溢出前评估栈余量。
- 任务创建失败、同步对象创建失败和调度器异常返回时，要先记录 heap Watch 变量，再写入对应 fault code。
- 栈溢出 hook 要把触发任务写入 `g_stackOverflowTaskHandle` 和 `g_stackOverflowTaskName`，便于故障现场定位。
- 关键运行路径至少保留一个递增计数，例如 task loop count、IRQ count、sample count。
- fault code 要集中定义，能区分 malloc 失败、栈溢出、同步对象创建失败、任务创建失败、调度器异常返回和 FreeRTOS assert 失败。
- UART printf 只能作为调试输出，不作为核心时序依赖。

## 5. 清单和文档

- 更新 `examples/examples.json`：
  - `schemaVersion` 保持 `1`。
  - 路径使用仓库相对路径和正斜杠。
  - 填写 `name`、`description`、`project`、`target`、`validationStatus`、`documentation`。
  - `documentation` 必须包含该示例自己的 `<示例目录>/README.md`。
- 新增或更新示例 README，至少包含数据流、硬件连接、构建方式、预期现象和关键配置。
- 更新 `docs/examples.md`、`docs/hardware-validation.md`、`docs/validation-status.md`，并确保验证状态矩阵的状态列和 `examples/examples.json` 一致。
- 加入 `examples/examples.json` 后，`scripts/check-repo.ps1` 会把该示例纳入第三方来源文件头检查。
- 如果示例能力超出当前边界，同步更新 `docs/known-limitations.md`。

## 6. 验证

开发阶段可以只构建目标示例：

```powershell
.\scripts\build-keil.ps1 -ExampleName <name> -CleanAfterBuild
```

提交前必须运行：

```powershell
.\scripts\check-repo.ps1
.\scripts\build-keil.ps1 -CleanAfterBuild
git diff --check
git status --short --ignored
```

通过标准：

- 仓库自检通过。
- 所有维护示例均为 `0 Error(s), 0 Warning(s)`。
- 没有提交 `例程/`、Keil 生成物、JLink 本机文件或临时归档。
- 如果没有完成板级硬件验收，`validationStatus` 只能保持 `build-verified` 或 `experimental`。
