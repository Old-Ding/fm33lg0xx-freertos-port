# 发布流程

本文记录本仓库的版本发布规则。目标是让每次发布都能追溯到具体 commit、构建结果、硬件验收状态和已知限制。

## 版本策略

- 使用语义化版本格式 `MAJOR.MINOR.PATCH`，Git tag 使用 `vMAJOR.MINOR.PATCH`。
- `PATCH` 用于文档修正、脚本修正或不改变示例行为的小修。
- `MINOR` 用于新增示例、扩展示例能力、调整 FreeRTOS 配置或增加维护脚本。
- `MAJOR` 仅在目录结构、支持芯片范围或示例工程兼容性发生破坏性变化时使用。
- `CHANGELOG.md` 中的 `Unreleased` 只记录尚未打 tag 的变化；发布时把对应条目移动到具体版本号和日期下面。

## 发布前门禁

发布前必须在本机运行：

```powershell
.\scripts\check-repo.ps1
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
git diff --check
git status --short --ignored
```

通过标准：

- 仓库自检通过。
- 所有 `examples/examples.json` 中维护的 Keil 示例都是 `0 Error(s), 0 Warning(s)`。
- `git diff --check` 没有空白错误。
- `git status --short --ignored` 只允许出现本地 ignored 的 `例程/` 等参考目录，不允许有未提交的源码、文档或生成物。
- `git ls-tree --name-only -r HEAD` 中不得出现 `例程/`、`MDK-ARM/Objects/`、`MDK-ARM/Listings/`、`.uvoptx`、`.uvguix.*`、JLink 日志或构建产物。
- `docs/validation-status.md` 必须反映每个示例的当前验证级别；未完成板级验收的示例只能标为 `build-verified` 或 `experimental`。
- `docs/known-limitations.md` 必须反映当前未覆盖的芯片、工具链、CI、硬件验证和 demo 能力边界。

## 硬件验收

至少需要确认：

- `gpio_blink_mdk`：PB4 LED 周期闪烁，`g_ledTaskCreateStatus == pdPASS`，`g_ledTaskLoopCount` 递增，`g_freertosFaultCode == 0`。
- `freertos_signal_adc_uart_mdk`：PB4 monitor 正常，PB12 下降沿能唤醒 GPIO task 和 ADC task，UART 输出 ADC 采样值，`g_freertosFaultCode == 0`。

如果发布时某个示例只完成构建验证、尚未完成板级硬件验证，必须在 release notes 中明确标注为“构建已验证，硬件待验证”，不能写成已验证。

## 发布步骤

1. 确认 `main` 已同步远端：

   ```powershell
   git status --short --branch
   git pull --ff-only origin main
   ```

2. 更新 `CHANGELOG.md`：

   ```text
   ## Unreleased

   ## 0.2.0 - YYYY-MM-DD

   - ...
   ```

3. 运行发布前门禁。

4. 提交 changelog：

   ```powershell
   git add CHANGELOG.md
   git commit -m "Prepare v0.2.0 release"
   ```

5. 创建带注释 tag：

   ```powershell
   git tag -a v0.2.0 -m "v0.2.0"
   ```

6. 推送 commit 和 tag：

   ```powershell
   git push origin main
   git push origin v0.2.0
   ```

7. 在 GitHub Release 中粘贴 release notes，至少包含：

   - 版本号和 commit。
   - 新增或变化的示例。
   - 本地 Keil 构建结果。
   - 硬件验收结果或待验证项。
   - 已知限制，优先引用 `docs/known-limitations.md`。

## 回滚和修正

- tag 推送前发现问题：直接修正 commit，再重新运行门禁。
- tag 推送后发现文档问题：发 `PATCH` 版本修正，不重写已公开 tag。
- tag 推送后发现示例行为问题：优先开 issue 记录复现环境、Watch 变量和构建输出，再用新版本修复。
