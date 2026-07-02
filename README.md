# FM33LG0xx FreeRTOS 移植

本仓库用于维护复旦微 FM33LG0xx 系列芯片上的 FreeRTOS 移植示例。当前基线是 `gpio_blink_mdk`：在原 GPIO 闪灯工程上加入 FreeRTOS，让 LED 闪烁、看门狗喂狗和掉电监测运行在 FreeRTOS 任务中。

## 当前状态

- 目标芯片：`FM33LG02X`，内核为 `Cortex-M0`。
- 工具链：Keil MDK / ARMCC5。
- FreeRTOS port：`FreeRTOS-Kernel-main/portable/RVDS/ARM_CM0`。
- 当前示例：`gpio_blink_mdk`，LED0 使用 `GPIOB PIN4`。
- 当前验证：Keil 工程已加入 `tasks.c`、`list.c`、`port.c`、`heap_4.c`，`SVC/PendSV/SysTick` 由 FreeRTOS 接管。

## 目录结构

```text
.
├── FreeRTOS-Kernel-main/       # FreeRTOS Kernel 源码
├── gpio_blink_mdk/             # FM33LG02X GPIO 闪灯移植示例
│   ├── Drivers/                # 复旦微/CMSIS 设备与外设驱动
│   ├── Inc/                    # 应用头文件与 FreeRTOSConfig.h
│   ├── MF-config/              # 复旦微配置生成文件
│   ├── MDK-ARM/                # Keil 工程文件
│   └── Src/                    # 应用源文件
├── docs/
│   └── porting-notes.md        # FreeRTOS 移植说明
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
4. `FreeRTOS-Kernel-main` 与 `gpio_blink_mdk` 保持当前相对位置，因为 Keil 工程使用相对路径引用 FreeRTOS。

Keil GUI 构建：

1. 打开 `gpio_blink_mdk/MDK-ARM/FM33LG0XX_Tester.uvprojx`。
2. 选择 target `Example`。
3. 执行 Build。

PowerShell 命令行构建示例：

```powershell
& '<Keil install path>\UV4\UV4.exe' -b '.\gpio_blink_mdk\MDK-ARM\FM33LG0XX_Tester.uvprojx' -t 'Example'
```

请把 `<Keil install path>` 替换为本机 Keil MDK 安装目录。

## 调试观察点

可以在 Keil Watch 中观察：

- `g_ledTaskCreateStatus`：`pdPASS` 表示 LED 任务创建成功。
- `g_ledTaskLoopCount`：持续增加表示调度器、SysTick 和 `vTaskDelay()` 正常工作。
- `g_freertosFaultCode`：`0` 为正常，`1` 为 malloc 失败，`2` 为任务栈溢出，`3` 为调度器启动失败或异常返回。

## 维护规则

- 不提交 `MDK-ARM/Objects`、`MDK-ARM/Listings`、`.uvoptx`、`.uvguix.*`、JLink 日志、`.axf/.hex/.map/.crf/.o/.d` 等生成产物。
- 改 RTOS 移植逻辑前，先确认启动链路、中断入口、SysTick 归属和厂商延时函数调用点。
- 调度器启动后，任务周期延时使用 `vTaskDelay()`；不要在任务里直接使用会重配 SysTick 的厂商 delay。
- 原创代码、文档和仓库元信息使用 MIT License；第三方代码保留原始许可证和文件头说明，来源见 `THIRD_PARTY_NOTICES.md`。
