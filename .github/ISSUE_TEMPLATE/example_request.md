---
name: Example request
about: 建议新增一个 FreeRTOS 示例
title: "[example] "
labels: enhancement
assignees: ""
---

## 示例目标

请说明这个示例要验证的 FreeRTOS 能力或外设组合。

## 建议范围

- 目标外设：
- 涉及 FreeRTOS API：
- 是否需要中断：
- 是否需要 UART/Watch 可观测输出：
- 是否依赖厂商完整 `例程/` 中的某个参考工程：

## 硬件连接

| 功能 | 引脚/外设 | 说明 |
| --- | --- | --- |
| | | |

## 验收标准

- [ ] Keil 构建 `0 Error(s), 0 Warning(s)`
- [ ] `.\scripts\check-repo.ps1` 通过
- [ ] 示例加入 `examples/examples.json`
- [ ] README 或 `docs/examples.md` 补充硬件引脚和预期现象
- [ ] 关键状态可通过 UART 或 Keil Watch 观察

## 其他说明
