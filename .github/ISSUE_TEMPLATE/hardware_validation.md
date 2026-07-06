---
name: Hardware validation
about: 记录某块板子的硬件验收结果
title: "[validation] "
labels: documentation
assignees: ""
---

## 验收对象

- 芯片/板卡：
- Git commit：
- 示例工程：
- Keil MDK 版本：
- DFP 设备包版本：
- 调试器：

## 构建结果

```text

```

## 接线

| 功能 | 实际连接 | 备注 |
| --- | --- | --- |
| LED0 / PB4 | | |
| GPIO interrupt / PB12 | | |
| ADC input / PD1 | | |
| UART0 / PA2 | | |

## 运行现象

- [ ] PB4 LED 周期翻转
- [ ] PB12 下降沿触发后 `g_gpioIrqCount` 递增
- [ ] GPIO task 唤醒后 `g_gpioTaskWakeCount` 递增
- [ ] ADC task 采样后 `g_adcSampleCount` 递增
- [ ] UART 输出启动日志和 ADC 采样值
- [ ] `g_freertosFaultCode == 0`

## UART 输出

```text

```

## Watch 变量

```text

```

## 结论

- [ ] 通过
- [ ] 未通过

补充说明：
