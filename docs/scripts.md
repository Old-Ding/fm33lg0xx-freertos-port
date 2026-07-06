# 维护脚本

本文记录仓库自有 PowerShell 脚本的职责、参数和失败条件。脚本入口都以仓库根目录为工作目录执行。

## 职责边界

- `scripts/check-repo.ps1`：检查仓库结构、示例清单、文档入口、第三方来源声明和禁止提交的生成物；不依赖 Keil。
- `scripts/build-keil.ps1`：调用 Keil `UV4.exe` 构建 `examples/examples.json` 中登记的示例，解析构建日志，并按参数清理 Keil 生成物。
- GitHub Actions 运行 `scripts/check-repo.ps1` 和提交范围空白检查；Keil 构建需要本机安装 MDK 和设备包，仍由本地脚本完成。

## 推荐命令

提交前先运行不依赖 Keil 的仓库自检：

```powershell
.\scripts\check-repo.ps1
```

查看当前脚本会构建哪些示例：

```powershell
.\scripts\build-keil.ps1 -ListExamples
```

开发阶段只验证一个示例：

```powershell
.\scripts\build-keil.ps1 -ExampleName freertos_signal_adc_uart_mdk -CleanAfterBuild
```

提交前验证全部示例并清理生成物：

```powershell
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
```

只清理 Keil 生成物：

```powershell
.\scripts\build-keil.ps1 -CleanOnly
```

## check-repo.ps1

`check-repo.ps1` 会检查：

- 已跟踪文件不得包含本地厂商完整 `例程/` 目录、Keil/JLink 生成物、归档包和临时文件。
- `.gitignore` 必须保留本地厂商完整 `例程/` 目录、Keil/JLink 生成物、归档包和临时文件的忽略规则。
- `.gitattributes` 必须保留项目文本 CRLF 和二进制产物属性规则。
- 项目自有文档、脚本、清单和 GitHub 模板必须是 UTF-8 无 BOM，换行必须是 CRLF。
- `examples/examples.json` 必须包含 `schemaVersion`、示例名称、说明、Keil 工程路径、Keil target、验证状态和文档入口。
- `examples/examples.json` 中的 Keil 工程路径和文档入口必须指向已被 Git 跟踪的仓库文件。
- `examples/examples.json` 中的 Keil target 必须存在于对应 `.uvprojx` 工程。
- 每个示例的文档入口必须包含该示例自己的 README。
- 每个示例工程必须引用根目录共享的 `FreeRTOS-Kernel-main`。
- 启用计数信号量的示例工程必须引用 `queue.c`。
- 第三方许可证、FreeRTOS 文件、厂商 FL Driver 文件头和 CMSIS SPDX 标识必须保留；示例根目录从 `examples/examples.json` 推导。
- `docs/validation-status.md` 必须在当前矩阵中逐示例记录 `examples/examples.json` 中的验证状态。
- `docs/known-limitations.md`、`docs/new-example-checklist.md` 和本文档必须保留关键维护信息。

脚本失败时会列出所有失败项。修复后重新运行同一个命令，直到输出 `Repository checks passed.`。

## build-keil.ps1

参数说明：

| 参数 | 说明 |
| --- | --- |
| `-UV4Path <path>` | 显式指定 `UV4.exe` 路径。 |
| `-ExampleName <name>` | 只构建指定示例；可传多个名称。名称来自 `examples/examples.json`。 |
| `-Mode Build` | 执行 Keil incremental build。 |
| `-Mode Rebuild` | 执行 Keil rebuild；默认值。 |
| `-ListExamples` | 列出示例名称、target、验证状态、工程路径和说明，不执行构建。 |
| `-CleanOnly` | 只清理所选示例的 Keil 生成物，不执行构建。 |
| `-CleanAfterBuild` | 构建通过后清理所选示例的 Keil 生成物。 |

参数组合规则：

- `-ListExamples` 只用于查看清单，不能和示例选择、构建、清理或 `UV4.exe` 路径参数组合。
- `-CleanOnly` 可与 `-ExampleName` 组合以只清理指定示例；不能和构建模式、`UV4.exe` 路径或 `-CleanAfterBuild` 组合。

`UV4.exe` 查找顺序：

1. `-UV4Path` 参数。
2. `KEIL_UV4` 环境变量。
3. `PATH` 中的 `UV4.exe`。
4. 常见安装路径，包括 `D:\keil\MDK542a\UV4\UV4.exe`、`C:\Keil_v5\UV4\UV4.exe`、`C:\Keil\UV4\UV4.exe`、`C:\Program Files\Keil_v5\UV4\UV4.exe` 和 `C:\Program Files (x86)\Keil_v5\UV4\UV4.exe`。

构建失败条件：

- 示例名称不在 `examples/examples.json` 中。
- 示例工程文件不存在。
- 找不到 `UV4.exe`。
- Keil 进程返回非 0 exit code。
- 构建日志不存在，或无法解析 `Error(s)` / `Warning(s)` 汇总。
- 构建日志早于本次 Keil 进程启动时间，说明脚本可能读到了旧日志。
- 任意示例出现 error 或 warning。
- 清理目标路径规范化后不在仓库或对应 Keil 工程目录内。

清理范围限定在每个示例的 `MDK-ARM` 目录：

- `Objects/`
- `Listings/`
- `*.uvoptx`
- `*.uvguix.*`
- `*.uvgui.*`
- `JLinkLog.txt`
- `JLinkSettings.ini`

## 发布前脚本门禁

发布或提交前建议按顺序运行：

```powershell
.\scripts\check-repo.ps1
.\scripts\build-keil.ps1 -UV4Path 'D:\keil\MDK542a\UV4\UV4.exe' -CleanAfterBuild
git diff --check
git status --short --ignored
```

`git status --short --ignored` 只应出现本地 ignored 的参考目录或临时文件，例如 `例程/`。如果出现未提交源码、文档或 Keil 生成物，先处理工作区再提交。
