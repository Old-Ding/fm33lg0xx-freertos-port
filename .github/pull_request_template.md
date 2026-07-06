## 变更内容

-

## 设计说明

请说明本次改动的数据流、调用链或职责边界。涉及 ISR、任务、状态机、延时、SysTick、FreeRTOS API 时必须写清楚唯一职责层。

## 验证

- [ ] 已运行 `.\scripts\check-repo.ps1`
- [ ] 已运行 `.\scripts\build-keil.ps1 -CleanAfterBuild`
- [ ] 如只验证单个示例，已说明原因和命令
- [ ] 已完成必要硬件验收，或已说明尚未具备硬件验证条件

## 示例清单

- [ ] 新增示例已加入 `examples/examples.json`
- [ ] Keil 工程仍引用根目录 `FreeRTOS-Kernel-main`
- [ ] 新增 FreeRTOS API 所需内核源文件已加入 Keil 工程

## 提交卫生

- [ ] 没有提交 `例程/` 厂商完整目录
- [ ] 没有提交 `Objects/`、`Listings/`、`.uvoptx`、`.uvguix.*`、JLink 日志或二进制生成物
- [ ] 文档已同步更新
