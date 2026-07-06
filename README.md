# FM33LG0xx FreeRTOS Port and Examples

本仓库用于维护复旦微 FM33LG0xx 系列芯片上的 FreeRTOS 移植和示例工程。当前策略是根目录统一维护一份 `FreeRTOS-Kernel-main`，各 Keil 示例通过相对路径引用内核源码，避免每个 demo 各复制一份 FreeRTOS。

## 当前状态

- 目标芯片：`FM33LG02X`，内核为 `Cortex-M0`。
- 工具链：Keil MDK / ARMCC5。
- FreeRTOS port：`FreeRTOS-Kernel-main/portable/RVDS/ARM_CM0`。
- 已验证基线：`gpio_blink_mdk`，用于验证 SysTick、任务调度、喂狗和 SVD 去抖处理。
- 扩展示例：`examples/freertos_signal_adc_uart_mdk`，用于演示任务、信号量、GPIO ISR、ADC 采样和 UART 调试输出。

## 示例列表

| 示例 | 说明 | Keil 工程 |
| --- | --- | --- |
| [`gpio_blink_mdk`](gpio_blink_mdk/README.md) | 最小 FreeRTOS 移植验证，LED 闪烁任务接管原主循环职责。 | `gpio_blink_mdk/MDK-ARM/FM33LG0XX_Tester.uvprojx` |
| [`examples/freertos_signal_adc_uart_mdk`](examples/freertos_signal_adc_uart_mdk/README.md) | 综合 demo：monitor task、GPIO 中断、二值/计数信号量、ADC task、UART printf。 | `examples/freertos_signal_adc_uart_mdk/MDK-ARM/FM33LG0XX_Tester.uvprojx` |

## 目录结构

```text
.
├── FreeRTOS-Kernel-main/                 # FreeRTOS Kernel 源码
├── gpio_blink_mdk/                       # 最小 GPIO 闪灯移植示例
├── examples/
│   ├── examples.json                     # 示例构建和文档元数据清单
│   └── freertos_signal_adc_uart_mdk/     # FreeRTOS 综合基础 demo
├── docs/
│   ├── architecture.md                   # 调用链和职责边界
│   ├── examples.md                       # 示例硬件和验证说明
│   ├── hardware-validation-record.md     # 硬件验收结果记录模板
│   ├── hardware-validation.md            # 板级硬件验收步骤
│   ├── known-limitations.md              # 当前支持边界和已知限制
│   ├── new-example-checklist.md          # 新增示例流程检查表
│   ├── porting-notes.md                  # FreeRTOS 移植说明
│   ├── release-process.md                # 版本发布和验收流程
│   ├── scripts.md                        # 本地维护脚本说明
│   └── validation-status.md              # 当前示例验证状态矩阵
├── scripts/
│   ├── build-keil.ps1                    # Keil 批量构建验证脚本
│   └── check-repo.ps1                    # 仓库结构和提交卫生自检脚本
├── .github/                              # GitHub Actions、issue 和 PR 模板
├── LICENSE
├── CONTRIBUTING.md
├── CHANGELOG.md
└── THIRD_PARTY_NOTICES.md
```

## 构建

前置条件：

1. Windows 11。
2. Keil MDK 5.x，ARMCC5 可用。
3. 已安装 `FMSH.FM33LG0XX_DFP.3.0.1` 设备包。
4. 保持仓库目录结构不变，因为 Keil 工程使用相对路径引用根目录 `FreeRTOS-Kernel-main`。

Keil GUI 构建：

1. 打开目标示例的 `MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`。
3. 执行 Build。

推荐用仓库脚本一次验证全部示例：

```powershell
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
```

如果 `UV4.exe` 已在 `PATH` 中，或已通过 `KEIL_UV4` 环境变量配置，也可以省略 `-UV4Path`。脚本会全量 rebuild 当前维护的示例，解析 Keil 日志，并在出现 warning 或 error 时失败。

脚本从 `examples/examples.json` 读取示例工程清单。后续新增 demo 时，把 Keil 工程路径、说明、验证状态和文档入口加入这个清单，提交前就能被统一构建和仓库自检覆盖。

脚本职责、参数、失败条件和清理范围见 [维护脚本说明](docs/scripts.md)。

仓库还提供不依赖 Keil 的结构自检，用于本地和 GitHub Actions 拦截厂商完整例程、Keil 生成物、坏掉的示例清单，并确认关键第三方许可证/来源文件仍在：

```powershell
.\scripts\check-repo.ps1
```

开发单个示例时可以只构建指定项：

```powershell
.\scripts\build-keil.ps1 -ListExamples
.\scripts\build-keil.ps1 -ExampleName freertos_signal_adc_uart_mdk -CleanAfterBuild
```

## 硬件验收

构建通过后，按 [硬件验收指南](docs/hardware-validation.md) 逐步确认下载运行、LED、GPIO 中断、ADC 采样、UART 输出和 Keil Watch 变量。

板级结果按 [硬件验收记录模板](docs/hardware-validation-record.md) 保留证据后，才能把示例状态升级为 `hardware-verified`。

示例调用链、ISR/task 职责边界和 SysTick 归属见 [架构说明](docs/architecture.md)。

当前示例的验证级别见 [验证状态](docs/validation-status.md)。`build-verified` 只表示 Keil 构建通过，不等同于板级硬件已验证。

当前支持边界和未覆盖能力见 [已知限制](docs/known-limitations.md)。

PowerShell 手工构建示例：

```powershell
& '<Keil install path>\UV4\UV4.exe' -b '.\gpio_blink_mdk\MDK-ARM\FM33LG0XX_Tester.uvprojx' -t 'Example'
& '<Keil install path>\UV4\UV4.exe' -b '.\examples\freertos_signal_adc_uart_mdk\MDK-ARM\FM33LG0XX_Tester.uvprojx' -t 'Example'
```

请把 `<Keil install path>` 替换为本机 Keil MDK 安装目录。

## 硬件引脚

| 示例 | 功能 | 引脚/外设 |
| --- | --- | --- |
| `gpio_blink_mdk` | LED0 | `GPIOB PIN4` |
| `freertos_signal_adc_uart_mdk` | LED0 monitor | `GPIOB PIN4` |
| `freertos_signal_adc_uart_mdk` | GPIO 下降沿中断 | `GPIOB PIN12` / `EXTI LINE7` |
| `freertos_signal_adc_uart_mdk` | ADC 采样 | `PD1` / `FL_ADC_EXTERNAL_CH1` |
| `freertos_signal_adc_uart_mdk` | UART printf | `UART0`，`PA2/PA3`，115200，8E1，TX only |

## 调试观察点

`gpio_blink_mdk` 可在 Keil Watch 中观察：

- `g_ledTaskCreateStatus`：`pdPASS` 表示 LED 任务创建成功。
- `g_ledTaskLoopCount`：持续增加表示调度器、SysTick 和 `vTaskDelay()` 正常工作。
- `g_ledTaskStackHighWaterMark`：LED task 剩余栈水位，用于评估任务栈余量。
- `g_freertosFaultCode`：`0` 为正常，`1` 为 malloc 失败，`2` 为任务栈溢出，`3` 为调度器启动失败或异常返回，`6` 为 FreeRTOS assert 失败。
- `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes`：`g_freertosFaultCode == 1` 时记录 heap 余量现场。
- `g_stackOverflowTaskHandle`、`g_stackOverflowTaskName`：`g_freertosFaultCode == 2` 时记录溢出任务现场。
- `g_freertosAssertFile`、`g_freertosAssertLine`：`g_freertosFaultCode == 6` 时记录 assert 触发位置。

`freertos_signal_adc_uart_mdk` 可观察：

- `g_monitorTaskCreateStatus`、`g_gpioTaskCreateStatus`、`g_adcTaskCreateStatus`：任务创建结果。
- `g_monitorTaskLoopCount`：monitor task 周期运行计数。
- `g_monitorTaskStackHighWaterMark`、`g_gpioTaskStackHighWaterMark`、`g_adcTaskStackHighWaterMark`：各任务剩余栈水位。
- `g_gpioIrqCount`、`g_gpioTaskWakeCount`：GPIO ISR 触发和任务唤醒计数。
- `g_gpioSemaphoreGiveFailCount`、`g_adcSemaphoreGiveFailCount`：PB12 触发过快时，观察 ISR 侧事件合并或计数信号量溢出。
- `g_adcSampleMv`、`g_adcSampleCount`：ADC 最近一次采样电压和采样次数。
- `g_freertosFaultCode`：`4` 表示同步对象创建失败，`5` 表示任务创建失败，`6` 表示 FreeRTOS assert 失败。
- `g_freertosHeapFreeBytes`、`g_freertosHeapMinimumEverFreeBytes`：`g_freertosFaultCode == 1` 时记录 heap 余量现场。
- `g_stackOverflowTaskHandle`、`g_stackOverflowTaskName`：`g_freertosFaultCode == 2` 时记录溢出任务现场。
- `g_freertosAssertFile`、`g_freertosAssertLine`：`g_freertosFaultCode == 6` 时记录 assert 触发位置。

## 维护规则

- 不提交 `例程/`、`MDK-ARM/Objects`、`MDK-ARM/Listings`、`.uvoptx`、`.uvguix.*`、JLink 日志、`.axf/.elf/.hex/.bin/.map/.lst/.crf/.o/.obj/.d/.dep/.lnp/.sct/.htm`、归档包和临时日志等本地或生成产物。
- 改 RTOS 移植逻辑前，先确认启动链路、中断入口、SysTick 归属和厂商延时函数调用点。
- 调度器启动后，任务周期延时使用 `vTaskDelay()`；不要在任务里直接使用会重配 SysTick 的厂商 delay。
- 新增 FreeRTOS API 时，同步检查 `FreeRTOSConfig.h` 和 Keil 工程中的内核源文件，例如信号量需要 `queue.c`。
- 新增示例时同步更新 `examples/examples.json`，让构建脚本自动覆盖新示例。
- 修改调用链、中断同步、SysTick 延时或任务职责时，同步更新 [架构说明](docs/architecture.md)。
- 新增示例前按 [新增示例 Checklist](docs/new-example-checklist.md) 先确认数据流、职责边界、文档和验证项。
- 本地检查和构建脚本的完整用法见 [维护脚本说明](docs/scripts.md)。
- 开发阶段可用 `-ExampleName` 快速验证单个示例；提交前仍运行 `.\scripts\build-keil.ps1 -CleanAfterBuild`，确认所有示例是 `0 Error(s), 0 Warning(s)`。
- 发布版本前按 [发布流程](docs/release-process.md) 更新 changelog、运行门禁并记录硬件验收状态。
- 原创代码、文档和仓库元信息使用 MIT License；第三方代码保留原始许可证和文件头说明，来源见 `THIRD_PARTY_NOTICES.md`。
