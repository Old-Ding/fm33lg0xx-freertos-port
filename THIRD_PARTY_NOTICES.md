# 第三方来源说明

本文件记录当前仓库中非原创代码的来源和许可证边界。根目录 `LICENSE` 只覆盖本项目原创代码、文档和仓库元信息；已经带有第三方版权或许可证声明的文件继续遵循其原始声明。

维护原则：

- 保留第三方文件原始文件头、版权声明、免责声明和 SPDX 标识。
- 只抽取当前示例必需的厂商 SDK/驱动文件，不提交本地完整 `例程/` 参考目录。
- 发布前运行 `.\scripts\check-repo.ps1`，确认关键许可证文件和代表性来源文件头仍存在。

## FreeRTOS Kernel

- 路径：`FreeRTOS-Kernel-main/`
- 来源：FreeRTOS Kernel 源码包。
- 许可证：MIT License，见 `FreeRTOS-Kernel-main/LICENSE.md`。
- 当前使用文件：`tasks.c`、`list.c`、`queue.c`、`portable/RVDS/ARM_CM0/port.c`、`portable/MemMang/heap_4.c`。
- 维护要求：保留 `FreeRTOS-Kernel-main/LICENSE.md` 和 FreeRTOS 原始目录结构；示例工程通过相对路径引用根目录统一内核，不在每个 demo 中复制内核源码。

## Fudan Micro FM33LG0xx SDK / FL Driver

- 路径：`gpio_blink_mdk/Drivers/`、`gpio_blink_mdk/MF-config/`、`examples/freertos_signal_adc_uart_mdk/Drivers/`、`examples/freertos_signal_adc_uart_mdk/MF-config/`、部分示例 `Src/` 和 `Inc/` 文件。
- 来源：复旦微 FM33LG0xx 示例工程与外设驱动。
- 许可证：相关源文件头部包含 SHANGHAI FUDAN MICROELECTRONICS GROUP CO., LTD. 版权和再分发条款。
- 维护要求：保留原文件头部版权和免责声明，不删除来源标识。

## Arm CMSIS

- 路径：`gpio_blink_mdk/Drivers/CMSIS/`、`examples/freertos_signal_adc_uart_mdk/Drivers/CMSIS/`
- 来源：CMSIS Cortex-M 设备支持文件。
- 许可证：相关源文件头部包含 Arm 版权和 SPDX 标识，例如 `SPDX-License-Identifier: Apache-2.0`。
- 维护要求：保留 CMSIS 原始文件头和 SPDX 标识，不把 CMSIS 文件归入本项目 MIT 许可证范围。

## Keil / FMSH Device Family Pack

- Keil 工程引用设备包：`FMSH.FM33LG0XX_DFP.3.0.1`。
- 本仓库不提交本机 Keil Pack 安装目录，也不提交 build log 中的本机许可证信息。
- 如果后续需要把 pack 文件或设备包内容完整再分发，必须先确认对应再分发授权。

## 项目原创部分许可证

项目原创移植说明、仓库说明、后续新增原创代码和仓库元信息使用 MIT License，见 `LICENSE`。该许可证不覆盖已经带有第三方版权或许可证声明的文件。
