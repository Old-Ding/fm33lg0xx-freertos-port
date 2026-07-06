# 第三方来源说明

本文件记录当前仓库中非原创代码的来源。公开发布前，应再次核对每个来源的许可证文本和再分发条件。

## FreeRTOS Kernel

- 路径：`FreeRTOS-Kernel-main/`
- 来源：FreeRTOS Kernel 源码包。
- 许可证：见 `FreeRTOS-Kernel-main/LICENSE.md`。
- 当前使用文件：`tasks.c`、`list.c`、`queue.c`、`portable/RVDS/ARM_CM0/port.c`、`portable/MemMang/heap_4.c`。

## Fudan Micro FM33LG0xx SDK / FL Driver

- 路径：`gpio_blink_mdk/Drivers/`、`gpio_blink_mdk/MF-config/`、`examples/freertos_signal_adc_uart_mdk/Drivers/`、`examples/freertos_signal_adc_uart_mdk/MF-config/`、部分示例 `Src/` 和 `Inc/` 文件。
- 来源：复旦微 FM33LG0xx 示例工程与外设驱动。
- 许可证：相关源文件头部包含 SHANGHAI FUDAN MICROELECTRONICS GROUP CO., LTD. 版权和再分发条款。
- 维护要求：保留原文件头部版权和免责声明，不删除来源标识。

## Arm CMSIS

- 路径：`gpio_blink_mdk/Drivers/CMSIS/`、`examples/freertos_signal_adc_uart_mdk/Drivers/CMSIS/`
- 来源：CMSIS Cortex-M 设备支持文件。
- 许可证：相关源文件头部包含 Arm 版权和 BSD 风格再分发条款。

## Keil / FMSH Device Family Pack

- Keil 工程引用设备包：`FMSH.FM33LG0XX_DFP.3.0.1`。
- 本仓库不提交本机 Keil Pack 安装目录，也不提交 build log 中的本机许可证信息。
- 如果后续需要把 pack 文件或设备包内容完整再分发，必须先确认对应再分发授权。

## 项目原创部分许可证

项目原创移植说明、仓库说明、后续新增原创代码和仓库元信息使用 MIT License，见 `LICENSE`。该许可证不覆盖已经带有第三方版权或许可证声明的文件。
