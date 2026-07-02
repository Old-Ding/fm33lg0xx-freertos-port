# Changelog

## Unreleased

- 清理 Keil/JLink 生成产物和本机状态文件。
- 将原中文示例目录整理为 `gpio_blink_mdk`。
- 新增 README、贡献说明、变更日志、第三方来源说明和 Git 忽略规则。
- 新增 LICENSE，明确原创部分使用 MIT，第三方文件保留原始许可证。

## 0.1.0 - 2026-06-12

- 在 FM33LG02X GPIO 闪灯工程中加入 FreeRTOS。
- 使用 RVDS `ARM_CM0` port，加入 `tasks.c`、`list.c`、`port.c`、`heap_4.c`。
- 通过 `FreeRTOSConfig.h` 将 `SVC_Handler`、`PendSV_Handler`、`SysTick_Handler` 映射到 FreeRTOS port。
- 将 LED 闪灯、喂狗和掉电监测迁移到 `LedBlinkTask`。
- 增加 `SVD_DelayUs()` 弱函数覆盖点，避免调度器启动后 SVD 去抖重配 SysTick。
