# 验证状态

本文记录仓库当前示例的验证级别。它只描述已经有证据支撑的状态；没有完成板级运行验收的示例，不标记为硬件已验证。

## 状态定义

| 状态 | 含义 |
| --- | --- |
| `build-verified` | 已通过 Keil 构建，结果为 `0 Error(s), 0 Warning(s)`；不声明板级硬件现象已经验证。 |
| `hardware-verified` | 已在目标板完成 LED、GPIO、ADC、UART 或对应示例所需硬件现象验证，并记录关键 Watch 变量。 |
| `experimental` | 示例或移植点仍处于探索阶段，不能作为发布基线。 |

## 当前矩阵

| 示例 | 状态 | 构建证据 | 硬件状态 | 备注 |
| --- | --- | --- | --- | --- |
| `gpio_blink_mdk` | `build-verified` | `.\scripts\build-keil.ps1 -CleanAfterBuild` 通过，`0 Error(s), 0 Warning(s)` | 待记录板级验收结果 | 硬件通过后需要记录 PB4 LED、`g_ledTaskCreateStatus`、`g_ledTaskLoopCount`、`g_freertosFaultCode`。 |
| `freertos_signal_adc_uart_mdk` | `build-verified` | `.\scripts\build-keil.ps1 -CleanAfterBuild` 通过，`0 Error(s), 0 Warning(s)` | 待记录板级验收结果 | 硬件通过后需要记录 PB4、PB12、ADC、UART 输出和关键 Watch 变量。 |

## 状态升级规则

- 从 `build-verified` 升级到 `hardware-verified` 前，先按 [硬件验收指南](hardware-validation.md) 完成对应示例的板级验证。
- 硬件验收结果应通过 GitHub hardware validation issue 或发布记录保留可追溯信息。
- 更新 `examples/examples.json` 中的 `validationStatus` 时，必须同步更新本文矩阵。
- 如果硬件条件变化导致结果不可复现，应降回 `build-verified` 或在 release notes 中标明限制。
