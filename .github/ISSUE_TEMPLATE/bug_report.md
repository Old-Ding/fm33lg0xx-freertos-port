---
name: Bug report
about: 报告构建、移植或示例运行问题
title: "[bug] "
labels: bug
assignees: ""
---

## 问题现象

请描述实际现象，以及你期望看到的现象。

## 复现环境

- 芯片/板卡：
- Keil MDK 版本：
- ARMCC 版本：
- DFP 设备包版本：
- 示例工程：
- Git commit：

## 复现步骤

1.
2.
3.

## 构建结果

请粘贴 `.\scripts\build-keil.ps1 -ExampleName <name> -CleanAfterBuild` 的关键输出。

```text

```

## 硬件现象

- LED：
- GPIO 中断输入：
- ADC 输入：
- UART 输出：

## Watch 变量

请按实际示例填写关键变量，例如 `g_freertosFaultCode`、任务创建状态、heap free/minimum、stack high-water mark、stack overflow task name、assert file/line、loop count、IRQ count、ADC count。

```text

```

## 已排查项

- [ ] 已运行 `.\scripts\check-repo.ps1`
- [ ] 已确认没有使用 Keil 生成物或本地 `例程/` 目录作为提交内容
- [ ] 已确认 `FreeRTOSConfig.h` 和 Keil 工程中的 FreeRTOS 源文件匹配
- [ ] 已确认硬件接线和串口参数与文档一致
