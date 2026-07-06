param()

$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$Failures = [System.Collections.Generic.List[string]]::new()
$VendorExamplesDirectoryName = -join ([char]0x4F8B, [char]0x7A0B)

function Add-Failure {
    param([string]$Message)

    $Failures.Add($Message)
}

function Invoke-Git {
    param([string[]]$Arguments)

    $output = & git -C $RepoRoot -c core.quotePath=false @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "git $($Arguments -join ' ') failed"
    }

    return @($output)
}

function Test-PathWithinDirectory {
    param(
        [string]$Path,
        [string]$Directory
    )

    $comparison = [System.StringComparison]::OrdinalIgnoreCase
    $normalizedDirectory = [System.IO.Path]::GetFullPath($Directory).TrimEnd('\', '/')
    $normalizedPath = [System.IO.Path]::GetFullPath($Path).TrimEnd('\', '/')

    if ($normalizedPath.Equals($normalizedDirectory, $comparison)) {
        return $true
    }

    return $normalizedPath.StartsWith($normalizedDirectory + [System.IO.Path]::DirectorySeparatorChar, $comparison)
}

function Resolve-RepoPath {
    param([string]$RelativePath)

    $fullPath = Join-Path -Path $RepoRoot -ChildPath $RelativePath
    $resolvedPath = (Resolve-Path -LiteralPath $fullPath).Path

    if (-not (Test-PathWithinDirectory -Path $resolvedPath -Directory $RepoRoot)) {
        throw "Path escapes repository: $RelativePath"
    }

    return $resolvedPath
}

function Get-RepoText {
    param([string]$Path)

    return Get-Content -LiteralPath $Path -Raw -Encoding UTF8
}

# Hook 检查必须限定在函数体内，避免被运行态监控路径里的同名赋值误判通过。
function Get-CFunctionBodyText {
    param(
        [string]$SourceText,
        [string]$FunctionName
    )

    $escapedFunctionName = [regex]::Escape($FunctionName)
    $match = [regex]::Match($SourceText, "\b$escapedFunctionName\s*\([^;{}]*\)\s*\{")
    if (-not $match.Success) {
        return ''
    }

    $bodyStart = $match.Index + $match.Length - 1
    $depth = 0
    for ($index = $bodyStart; $index -lt $SourceText.Length; $index++) {
        if ($SourceText[$index] -eq '{') {
            $depth++
        } elseif ($SourceText[$index] -eq '}') {
            $depth--
            if ($depth -eq 0) {
                return $SourceText.Substring($bodyStart, $index - $bodyStart + 1)
            }
        }
    }

    return ''
}

function Test-GitTrackedPath {
    param(
        [string]$RelativePath,
        [string]$Description
    )

    $gitPath = $RelativePath -replace '\\', '/'
    & git -C $RepoRoot -c core.quotePath=false ls-files --error-unmatch -- $gitPath > $null 2> $null
    if ($LASTEXITCODE -ne 0) {
        Add-Failure "$RelativePath is not tracked by git: $Description"
    }
}

function Get-ManifestExampleRoots {
    $manifestPath = Resolve-RepoPath -RelativePath 'examples\examples.json'

    try {
        $manifest = Get-RepoText -Path $manifestPath | ConvertFrom-Json
    } catch {
        Add-Failure "examples/examples.json is not valid JSON: $($_.Exception.Message)"
        return @()
    }

    $roots = [ordered]@{}
    foreach ($entry in @($manifest.examples)) {
        if (-not $entry.project) {
            continue
        }

        $projectRelative = [string]$entry.project
        if (($projectRelative -match '\\') -or
            [System.IO.Path]::IsPathRooted($projectRelative) -or
            ($projectRelative -match '(^|/)\.\.(/|$)')) {
            continue
        }

        try {
            $projectPath = Resolve-RepoPath -RelativePath $projectRelative
        } catch {
            continue
        }

        $projectDir = Split-Path -Parent $projectPath
        $exampleRoot = Split-Path -Parent $projectDir
        $exampleRootRelative = $exampleRoot.Substring($RepoRoot.Length).TrimStart('\', '/') -replace '\\', '/'

        if ($exampleRootRelative -and (-not $roots.Contains($exampleRootRelative))) {
            $roots[$exampleRootRelative] = $true
        }
    }

    return @($roots.Keys)
}

function Test-BlockedTrackedFile {
    param([string]$Path)

    $normalized = $Path -replace '\\', '/'

    $blockedPathPatterns = @(
        @{ Pattern = '^' + [regex]::Escape($VendorExamplesDirectoryName) + '/'; Reason = 'vendor reference examples must stay local' },
        @{ Pattern = '(^|/)MDK-ARM/Objects/'; Reason = 'Keil Objects output must not be tracked' },
        @{ Pattern = '(^|/)MDK-ARM/Listings/'; Reason = 'Keil Listings output must not be tracked' },
        @{ Pattern = '(^|/)MDK-ARM/[^/]+\.uvoptx$'; Reason = 'Keil user option file must not be tracked' },
        @{ Pattern = '(^|/)MDK-ARM/[^/]+\.uvgui(x)?\.[^/]+$'; Reason = 'Keil user GUI state must not be tracked' },
        @{ Pattern = '(^|/)MDK-ARM/JLinkLog\.txt$'; Reason = 'JLink log must not be tracked' },
        @{ Pattern = '(^|/)MDK-ARM/JLinkSettings\.ini$'; Reason = 'local JLink settings must not be tracked' }
    )

    foreach ($blocked in $blockedPathPatterns) {
        if ($normalized -match $blocked.Pattern) {
            Add-Failure "${Path}: $($blocked.Reason)"
        }
    }

    if ($normalized -match '\.(axf|elf|hex|bin|exe|map|lst|o|obj|d|crf|dep|lnp|sct|htm|zip|7z|rar|pack|log|tmp)$') {
        Add-Failure "${Path}: generated artifact or local archive must not be tracked"
    }
}

function Test-GitignorePolicy {
    $requiredPatterns = @(
        "/$VendorExamplesDirectoryName/",
        '**/MDK-ARM/Objects/',
        '**/MDK-ARM/Listings/',
        '**/MDK-ARM/*.uvguix.*',
        '**/MDK-ARM/*.uvgui.*',
        '**/MDK-ARM/*.uvoptx',
        '**/MDK-ARM/JLinkLog.txt',
        '**/MDK-ARM/JLinkSettings.ini',
        '*.axf',
        '*.elf',
        '*.hex',
        '*.bin',
        '*.exe',
        '*.map',
        '*.lst',
        '*.crf',
        '*.o',
        '*.obj',
        '*.d',
        '*.dep',
        '*.lnp',
        '*.sct',
        '*.htm',
        '*.pack',
        '*.zip',
        '*.7z',
        '*.rar',
        '*.log',
        '*.tmp'
    )

    try {
        $gitignorePath = Resolve-RepoPath -RelativePath '.gitignore'
    } catch {
        Add-Failure '.gitignore is missing'
        return
    }

    $configuredPatterns = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($line in (Get-Content -LiteralPath $gitignorePath -Encoding UTF8)) {
        $trimmed = $line.Trim()
        if (($trimmed.Length -eq 0) -or $trimmed.StartsWith('#')) {
            continue
        }
        [void]$configuredPatterns.Add($trimmed)
    }

    foreach ($pattern in $requiredPatterns) {
        if (-not $configuredPatterns.Contains($pattern)) {
            Add-Failure ".gitignore must ignore $pattern"
        }
    }
}

function Test-GitattributesPolicy {
    $requiredAttributes = @(
        '* text=auto eol=crlf',
        '*.c text eol=crlf',
        '*.h text eol=crlf',
        '*.s text eol=crlf',
        '*.ld text eol=crlf',
        '*.json text eol=crlf',
        '*.yml text eol=crlf',
        '*.yaml text eol=crlf',
        '*.md text eol=crlf',
        '*.ps1 text eol=crlf',
        '*.txt text eol=crlf',
        '*.uvprojx text eol=crlf',
        '*.scvd text eol=crlf',
        '*.zip binary',
        '*.pack binary',
        '*.axf binary',
        '*.elf binary',
        '*.hex text eol=crlf',
        '*.bin binary'
    )

    try {
        $gitattributesPath = Resolve-RepoPath -RelativePath '.gitattributes'
    } catch {
        Add-Failure '.gitattributes is missing'
        return
    }

    $configuredAttributes = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::Ordinal)
    foreach ($line in (Get-Content -LiteralPath $gitattributesPath -Encoding UTF8)) {
        $trimmed = $line.Trim()
        if (($trimmed.Length -eq 0) -or $trimmed.StartsWith('#')) {
            continue
        }
        [void]$configuredAttributes.Add($trimmed)
    }

    foreach ($attribute in $requiredAttributes) {
        if (-not $configuredAttributes.Contains($attribute)) {
            Add-Failure ".gitattributes must contain $attribute"
        }
    }
}

function Test-OwnedTextFileFormat {
    param([string]$Path)

    $normalized = $Path -replace '\\', '/'
    $ownedTextPatterns = @(
        '^(\.gitattributes|\.gitignore|README\.md|CONTRIBUTING\.md|CHANGELOG\.md|LICENSE|THIRD_PARTY_NOTICES\.md)$',
        '^docs/.*\.md$',
        '^scripts/.*\.ps1$',
        '^examples/examples\.json$',
        '^\.github/.*\.(md|yml|yaml)$',
        '^gpio_blink_mdk/README\.md$',
        '^examples/[^/]+/README\.md$'
    )

    $isOwnedText = $false
    foreach ($pattern in $ownedTextPatterns) {
        if ($normalized -match $pattern) {
            $isOwnedText = $true
            break
        }
    }

    if (-not $isOwnedText) {
        return
    }

    $filePath = Resolve-RepoPath -RelativePath $Path
    $bytes = [System.IO.File]::ReadAllBytes($filePath)

    if (($bytes.Length -ge 3) -and
        ($bytes[0] -eq 0xEF) -and
        ($bytes[1] -eq 0xBB) -and
        ($bytes[2] -eq 0xBF)) {
        Add-Failure "${Path}: project-owned text file must be UTF-8 without BOM"
    }

    for ($index = 0; $index -lt $bytes.Length; $index++) {
        if (($bytes[$index] -eq 0x0A) -and
            (($index -eq 0) -or ($bytes[$index - 1] -ne 0x0D))) {
            Add-Failure "${Path}: project-owned text file must use CRLF line endings"
            break
        }
    }
}

function Test-ExampleManifest {
    $manifestPath = Join-Path -Path $RepoRoot -ChildPath 'examples\examples.json'

    if (-not (Test-Path -LiteralPath $manifestPath)) {
        Add-Failure 'examples/examples.json is missing'
        return
    }

    try {
        $manifest = Get-RepoText -Path $manifestPath | ConvertFrom-Json
    } catch {
        Add-Failure "examples/examples.json is not valid JSON: $($_.Exception.Message)"
        return
    }

    if (-not $manifest.examples) {
        Add-Failure 'examples/examples.json must define examples'
        return
    }

    if ([int]$manifest.schemaVersion -ne 1) {
        Add-Failure 'examples/examples.json schemaVersion must be 1'
    }

    $names = @{}
    $allowedExampleFields = @('name', 'description', 'project', 'target', 'validationStatus', 'documentation')
    foreach ($entry in $manifest.examples) {
        foreach ($field in $entry.PSObject.Properties.Name) {
            if ($allowedExampleFields -notcontains $field) {
                Add-Failure "example entry contains unsupported field: $field"
            }
        }

        if (-not $entry.name) {
            Add-Failure 'example entry is missing name'
            continue
        }

        $name = [string]$entry.name
        if ($names.ContainsKey($name)) {
            Add-Failure "duplicate example name: $name"
        } else {
            $names[$name] = $true
        }

        if ($name -notmatch '^[A-Za-z0-9_.-]+$') {
            Add-Failure "example name uses unsupported characters: $name"
        }

        if (-not $entry.description) {
            Add-Failure "example '$name' is missing description"
        }

        $target = [string]$entry.target
        if (-not $target) {
            Add-Failure "example '$name' is missing target"
        }

        $allowedValidationStatus = @('build-verified', 'hardware-verified', 'experimental')
        if (-not $entry.validationStatus) {
            Add-Failure "example '$name' is missing validationStatus"
        } elseif ($allowedValidationStatus -notcontains [string]$entry.validationStatus) {
            Add-Failure "example '$name' validationStatus must be one of: $($allowedValidationStatus -join ', ')"
        }

        if (-not $entry.documentation) {
            Add-Failure "example '$name' is missing documentation entries"
        } else {
            $documents = @{}
            foreach ($documentRelative in @($entry.documentation)) {
                if ([string]::IsNullOrWhiteSpace([string]$documentRelative)) {
                    Add-Failure "example '$name' has empty documentation path"
                    continue
                }

                if ([string]$documentRelative -match '\\') {
                    Add-Failure "example '$name' documentation path must use forward slashes: $documentRelative"
                }

                if ([System.IO.Path]::IsPathRooted([string]$documentRelative) -or
                    ([string]$documentRelative -match '(^|/)\.\.(/|$)')) {
                    Add-Failure "example '$name' documentation path must stay repository-relative: $documentRelative"
                    continue
                }

                if ($documents.ContainsKey([string]$documentRelative)) {
                    Add-Failure "example '$name' has duplicate documentation path: $documentRelative"
                } else {
                    $documents[[string]$documentRelative] = $true
                }

                try {
                    $documentPath = Resolve-RepoPath -RelativePath ([string]$documentRelative)
                } catch {
                    Add-Failure "example '$name' documentation path is invalid: $documentRelative"
                    continue
                }

                Test-GitTrackedPath -RelativePath ([string]$documentRelative) `
                    -Description "example '$name' documentation entry"

                $documentText = Get-RepoText -Path $documentPath
                if ($documentText -notmatch [regex]::Escape($name)) {
                    Add-Failure "example '$name' documentation does not mention example name: $documentRelative"
                }
            }
        }

        if (-not $entry.project) {
            Add-Failure "example '$name' is missing project"
            continue
        }

        $projectRelative = [string]$entry.project
        if ($projectRelative -match '\\') {
            Add-Failure "example '$name' project path must use forward slashes: $projectRelative"
        }

        if ([System.IO.Path]::IsPathRooted($projectRelative) -or
            ($projectRelative -match '(^|/)\.\.(/|$)')) {
            Add-Failure "example '$name' project path must stay repository-relative: $projectRelative"
        }

        if ($projectRelative -notmatch '\.uvprojx$') {
            Add-Failure "example '$name' project must be a .uvprojx file: $projectRelative"
        }

        try {
            $projectPath = Resolve-RepoPath -RelativePath $projectRelative
        } catch {
            Add-Failure "example '$name' project path is invalid: $projectRelative"
            continue
        }

        Test-GitTrackedPath -RelativePath $projectRelative `
            -Description "example '$name' Keil project"

        $projectText = Get-RepoText -Path $projectPath
        if ($projectText -notmatch 'FreeRTOS-Kernel-main') {
            Add-Failure "example '$name' must reference the shared FreeRTOS-Kernel-main"
        }

        try {
            $projectXml = [xml]$projectText
            $targetNames = @($projectXml.Project.Targets.Target | ForEach-Object { [string]$_.TargetName })
            if ($target -and ($targetNames -notcontains $target)) {
                Add-Failure "example '$name' target '$target' is not defined in Keil project: $projectRelative"
            }
        } catch {
            Add-Failure "example '$name' Keil project is not valid XML: $projectRelative"
        }

        $projectDir = Split-Path -Parent $projectPath
        $exampleRoot = Split-Path -Parent $projectDir
        $exampleRootRelative = $exampleRoot.Substring($RepoRoot.Length).TrimStart('\', '/') -replace '\\', '/'
        $exampleReadmeRelative = "$exampleRootRelative/README.md"
        if (@($entry.documentation) -notcontains $exampleReadmeRelative) {
            Add-Failure "example '$name' documentation must include example README: $exampleReadmeRelative"
        }

        $configPath = Join-Path -Path $exampleRoot -ChildPath 'Inc\FreeRTOSConfig.h'

        if (Test-Path -LiteralPath $configPath) {
            $configText = Get-RepoText -Path $configPath

            if ($configText -notmatch '#define\s+configUSE_MALLOC_FAILED_HOOK\s+1') {
                Add-Failure "example '$name' must enable configUSE_MALLOC_FAILED_HOOK"
            }

            if ($configText -notmatch '#define\s+configCHECK_FOR_STACK_OVERFLOW\s+2') {
                Add-Failure "example '$name' must enable configCHECK_FOR_STACK_OVERFLOW level 2"
            }

            if ($configText -notmatch '#define\s+configASSERT\s*\(') {
                Add-Failure "example '$name' must define configASSERT"
            } elseif ($configText -notmatch 'FreeRTOS_AssertFailed\s*\(\s*__FILE__\s*,\s*__LINE__\s*\)') {
                Add-Failure "example '$name' configASSERT must call FreeRTOS_AssertFailed with __FILE__ and __LINE__"
            }

            $requiredHandlerMappings = @(
                @{ Pattern = '#define\s+vPortSVCHandler\s+SVC_Handler'; Name = 'vPortSVCHandler' },
                @{ Pattern = '#define\s+xPortPendSVHandler\s+PendSV_Handler'; Name = 'xPortPendSVHandler' },
                @{ Pattern = '#define\s+xPortSysTickHandler\s+SysTick_Handler'; Name = 'xPortSysTickHandler' }
            )
            foreach ($mapping in $requiredHandlerMappings) {
                if ($configText -notmatch $mapping.Pattern) {
                    Add-Failure "example '$name' FreeRTOSConfig.h must map $($mapping.Name) to the startup handler"
                }
            }

            # 信号量依赖 queue.c；这里检查工程引用，避免新增 API 后只改配置不改 Keil 工程。
            if (($configText -match '#define\s+configUSE_COUNTING_SEMAPHORES\s+1') -and
                ($projectText -notmatch 'queue\.c')) {
                Add-Failure "example '$name' enables counting semaphores but project does not reference queue.c"
            }

            $ownedSourceFiles = @()
            foreach ($sourceDirectory in @('Src', 'Inc')) {
                $sourceRoot = Join-Path -Path $exampleRoot -ChildPath $sourceDirectory
                if (Test-Path -LiteralPath $sourceRoot) {
                    $ownedSourceFiles += Get-ChildItem -LiteralPath $sourceRoot -Recurse -File -Include '*.c', '*.h'
                }
            }

            $usesStackHighWaterMark = $false
            $hasMallocFailedHook = $false
            $hasStackOverflowHook = $false
            $recordsStackOverflowTaskHandle = $false
            $recordsStackOverflowTaskName = $false
            $hasAssertFailedHandler = $false
            $recordsAssertFile = $false
            $recordsAssertLine = $false
            $recordsAssertFaultCode = $false
            $assertHandlerDisablesInterrupts = $false
            $recordsMallocHookFreeHeap = $false
            $recordsMallocHookMinimumHeap = $false
            $recordsRuntimeFreeHeap = $false
            $recordsRuntimeMinimumHeap = $false
            foreach ($sourceFile in $ownedSourceFiles) {
                $sourceText = Get-RepoText -Path $sourceFile.FullName
                if ($sourceText -match '\buxTaskGetStackHighWaterMark\b') {
                    $usesStackHighWaterMark = $true
                }

                if ($sourceText -match '\bvApplicationMallocFailedHook\s*\([^)]*\)\s*\{') {
                    $hasMallocFailedHook = $true
                }

                if ($sourceText -match '\bvApplicationStackOverflowHook\s*\([^)]*\)\s*\{') {
                    $hasStackOverflowHook = $true
                }

                if ($sourceText -match '\bg_stackOverflowTaskHandle\s*=\s*xTask\s*;') {
                    $recordsStackOverflowTaskHandle = $true
                }

                if ($sourceText -match '\bg_stackOverflowTaskName\s*=\s*pcTaskName\s*;') {
                    $recordsStackOverflowTaskName = $true
                }

                if ($sourceText -match '\bFreeRTOS_AssertFailed\s*\([^)]*\)\s*\{') {
                    $hasAssertFailedHandler = $true
                }

                if ($sourceText -match '\bg_freertosAssertFile\s*=\s*pcFile\s*;') {
                    $recordsAssertFile = $true
                }

                if ($sourceText -match '\bg_freertosAssertLine\s*=\s*ulLine\s*;') {
                    $recordsAssertLine = $true
                }

                if ($sourceText -match '\bg_freertosFaultCode\s*=\s*FREERTOS_FAULT_ASSERT\s*;') {
                    $recordsAssertFaultCode = $true
                }

                if ($sourceText -match '\b__disable_irq\s*\(\s*\)\s*;') {
                    $assertHandlerDisablesInterrupts = $true
                }

                $mallocFailedHookBody = Get-CFunctionBodyText -SourceText $sourceText -FunctionName 'vApplicationMallocFailedHook'
                if ($mallocFailedHookBody -match '\bg_freertosHeapFreeBytes\s*=\s*xPortGetFreeHeapSize\s*\(\s*\)\s*;') {
                    $recordsMallocHookFreeHeap = $true
                }

                if ($mallocFailedHookBody -match '\bg_freertosHeapMinimumEverFreeBytes\s*=\s*xPortGetMinimumEverFreeHeapSize\s*\(\s*\)\s*;') {
                    $recordsMallocHookMinimumHeap = $true
                }

                if ($sourceText -match '\buxTaskGetStackHighWaterMark\s*\([^;]*\)\s*;\s*\r?\n\s*g_freertosHeapFreeBytes\s*=\s*xPortGetFreeHeapSize\s*\(\s*\)\s*;') {
                    $recordsRuntimeFreeHeap = $true
                }

                if ($sourceText -match '\buxTaskGetStackHighWaterMark\s*\([^;]*\)\s*;\s*(?:\r?\n\s*g_freertosHeapFreeBytes\s*=\s*xPortGetFreeHeapSize\s*\(\s*\)\s*;)?\s*\r?\n\s*g_freertosHeapMinimumEverFreeBytes\s*=\s*xPortGetMinimumEverFreeHeapSize\s*\(\s*\)\s*;') {
                    $recordsRuntimeMinimumHeap = $true
                }
            }

            # Watch 变量依赖 FreeRTOS INCLUDE 开关；缺配置会在维护新示例时变成编译期隐性错误。
            if ($usesStackHighWaterMark -and
                ($configText -notmatch '#define\s+INCLUDE_uxTaskGetStackHighWaterMark\s+1')) {
                Add-Failure "example '$name' uses uxTaskGetStackHighWaterMark but FreeRTOSConfig.h does not enable INCLUDE_uxTaskGetStackHighWaterMark"
            }

            if (-not $hasMallocFailedHook) {
                Add-Failure "example '$name' must implement vApplicationMallocFailedHook"
            }

            if (-not $recordsMallocHookFreeHeap) {
                Add-Failure "example '$name' malloc failed hook must record xPortGetFreeHeapSize in g_freertosHeapFreeBytes"
            }

            if (-not $recordsMallocHookMinimumHeap) {
                Add-Failure "example '$name' malloc failed hook must record xPortGetMinimumEverFreeHeapSize in g_freertosHeapMinimumEverFreeBytes"
            }

            if (-not $recordsRuntimeFreeHeap) {
                Add-Failure "example '$name' runtime monitor must record xPortGetFreeHeapSize in g_freertosHeapFreeBytes"
            }

            if (-not $recordsRuntimeMinimumHeap) {
                Add-Failure "example '$name' runtime monitor must record xPortGetMinimumEverFreeHeapSize in g_freertosHeapMinimumEverFreeBytes"
            }

            if (-not $hasStackOverflowHook) {
                Add-Failure "example '$name' must implement vApplicationStackOverflowHook"
            }

            if (-not $recordsStackOverflowTaskHandle) {
                Add-Failure "example '$name' stack overflow hook must record xTask in g_stackOverflowTaskHandle"
            }

            if (-not $recordsStackOverflowTaskName) {
                Add-Failure "example '$name' stack overflow hook must record pcTaskName in g_stackOverflowTaskName"
            }

            if (-not $hasAssertFailedHandler) {
                Add-Failure "example '$name' must implement FreeRTOS_AssertFailed"
            }

            if (-not $recordsAssertFile) {
                Add-Failure "example '$name' FreeRTOS_AssertFailed must record pcFile in g_freertosAssertFile"
            }

            if (-not $recordsAssertLine) {
                Add-Failure "example '$name' FreeRTOS_AssertFailed must record ulLine in g_freertosAssertLine"
            }

            if (-not $recordsAssertFaultCode) {
                Add-Failure "example '$name' FreeRTOS_AssertFailed must set g_freertosFaultCode to FREERTOS_FAULT_ASSERT"
            }

            if (-not $assertHandlerDisablesInterrupts) {
                Add-Failure "example '$name' FreeRTOS_AssertFailed must disable interrupts before trapping"
            }
        } else {
            Add-Failure "example '$name' is missing Inc/FreeRTOSConfig.h"
        }
    }
}

function Test-FileContains {
    param(
        [string]$RelativePath,
        [string]$Pattern,
        [string]$Description
    )

    try {
        $path = Resolve-RepoPath -RelativePath $RelativePath
    } catch {
        Add-Failure "$RelativePath is missing: $Description"
        return
    }

    $content = Get-RepoText -Path $path
    if ($content -notmatch $Pattern) {
        Add-Failure "$RelativePath does not contain expected notice: $Description"
    }
}

function Test-ThirdPartyProvenance {
    Test-FileContains -RelativePath 'LICENSE' `
        -Pattern 'Third-party code, vendor SDK files, CMSIS files, FreeRTOS Kernel files' `
        -Description 'root license must keep third-party boundary statement'

    Test-FileContains -RelativePath 'THIRD_PARTY_NOTICES.md' `
        -Pattern 'FreeRTOS Kernel' `
        -Description 'third-party notices must document FreeRTOS'

    Test-FileContains -RelativePath 'FreeRTOS-Kernel-main\LICENSE.md' `
        -Pattern 'MIT License' `
        -Description 'FreeRTOS license file must be preserved'

    $requiredFreeRtosFiles = @(
        'FreeRTOS-Kernel-main\tasks.c',
        'FreeRTOS-Kernel-main\list.c',
        'FreeRTOS-Kernel-main\queue.c',
        'FreeRTOS-Kernel-main\portable\RVDS\ARM_CM0\port.c',
        'FreeRTOS-Kernel-main\portable\MemMang\heap_4.c'
    )
    foreach ($relativePath in $requiredFreeRtosFiles) {
        try {
            [void](Resolve-RepoPath -RelativePath $relativePath)
        } catch {
            Add-Failure "$relativePath is missing: shared FreeRTOS source used by examples"
        }
    }

    $exampleRoots = Get-ManifestExampleRoots
    if ($exampleRoots.Count -eq 0) {
        Add-Failure 'examples/examples.json does not expose any example roots for third-party provenance checks'
        return
    }

    foreach ($exampleRoot in $exampleRoots) {
        Test-FileContains -RelativePath "$exampleRoot\Drivers\FM33LG0xx_FL_Driver\Inc\fm33lg0xx_fl.h" `
            -Pattern 'SHANGHAI FUDAN MICROELECTRONICS GROUP CO\., LTD\.' `
            -Description 'Fudan Micro FL Driver copyright header must be preserved'

        Test-FileContains -RelativePath "$exampleRoot\Drivers\FM33LG0xx_FL_Driver\Inc\fm33lg0xx_fl.h" `
            -Pattern 'Redistribution and use in source and binary forms' `
            -Description 'Fudan Micro redistribution terms must be preserved'

        Test-FileContains -RelativePath "$exampleRoot\Drivers\CMSIS\Device\FM\FM33xx\Include\core_cm0plus.h" `
            -Pattern 'SPDX-License-Identifier:\s+Apache-2\.0' `
            -Description 'CMSIS SPDX license identifier must be preserved'
    }
}

function Test-KnownLimitationsDocument {
    Test-FileContains -RelativePath 'docs\known-limitations.md' `
        -Pattern 'build-verified' `
        -Description 'known limitations must document current validation boundary'

    Test-FileContains -RelativePath 'docs\known-limitations.md' `
        -Pattern 'Keil/ARMCC5' `
        -Description 'known limitations must document local Keil build boundary'

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/known-limitations\.md' `
        -Description 'README must link known limitations'
}

function Test-NewExampleChecklistDocument {
    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'examples/examples\.json' `
        -Description 'new example checklist must require manifest updates'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'queue\.c' `
        -Description 'new example checklist must mention FreeRTOS queue source dependency'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'configUSE_MALLOC_FAILED_HOOK' `
        -Description 'new example checklist must mention malloc failed hook'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'g_freertosHeapMinimumEverFreeBytes' `
        -Description 'new example checklist must mention malloc failed heap watch variable'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'configCHECK_FOR_STACK_OVERFLOW' `
        -Description 'new example checklist must mention stack overflow hook'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'configASSERT' `
        -Description 'new example checklist must mention FreeRTOS assert configuration'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'g_freertosAssertLine' `
        -Description 'new example checklist must mention FreeRTOS assert line watch variable'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'xPortSysTickHandler' `
        -Description 'new example checklist must mention FreeRTOS exception handler mapping'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'g_stackOverflowTaskName' `
        -Description 'new example checklist must mention stack overflow task name watch variable'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'portYIELD_FROM_ISR' `
        -Description 'new example checklist must document ISR yield pattern'

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/new-example-checklist\.md' `
        -Description 'README must link new example checklist'
}

function Test-ArchitectureDocument {
    Test-FileContains -RelativePath 'docs\architecture.md' `
        -Pattern 'SysTick' `
        -Description 'architecture document must describe SysTick ownership'

    Test-FileContains -RelativePath 'docs\architecture.md' `
        -Pattern 'portYIELD_FROM_ISR' `
        -Description 'architecture document must describe ISR yield pattern'

    Test-FileContains -RelativePath 'docs\architecture.md' `
        -Pattern 'queue\.c' `
        -Description 'architecture document must describe FreeRTOS source dependencies'

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/architecture\.md' `
        -Description 'README must link architecture document'

    Test-FileContains -RelativePath 'docs\new-example-checklist.md' `
        -Pattern 'docs/architecture\.md' `
        -Description 'new example checklist must reference architecture document'
}

function Test-ScriptsDocument {
    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'build-keil\.ps1' `
        -Description 'script document must describe Keil build script'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'check-repo\.ps1' `
        -Description 'script document must describe repository check script'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern '-CleanAfterBuild' `
        -Description 'script document must describe generated-output cleanup flow'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'configUSE_MALLOC_FAILED_HOOK' `
        -Description 'script document must describe FreeRTOS hook checks'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'g_freertosHeapMinimumEverFreeBytes' `
        -Description 'script document must describe malloc failed heap watch variable checks'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'xPortSysTickHandler' `
        -Description 'script document must describe FreeRTOS exception handler mapping checks'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'configASSERT' `
        -Description 'script document must describe FreeRTOS assert checks'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'g_freertosAssertLine' `
        -Description 'script document must describe FreeRTOS assert watch variable checks'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'g_stackOverflowTaskName' `
        -Description 'script document must describe stack overflow watch variable checks'

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/scripts\.md' `
        -Description 'README must link script document'
}

function Test-ChangelogDocument {
    Test-FileContains -RelativePath 'CHANGELOG.md' `
        -Pattern '(?m)^## Unreleased\s*$' `
        -Description 'changelog must keep an Unreleased section'

    Test-FileContains -RelativePath 'CHANGELOG.md' `
        -Pattern '(?m)^## \d+\.\d+\.\d+ - \d{4}-\d{2}-\d{2}\s*$' `
        -Description 'changelog must keep dated semantic-version release sections'

    Test-FileContains -RelativePath 'docs\release-process.md' `
        -Pattern 'CHANGELOG\.md' `
        -Description 'release process must describe changelog handling'

    Test-FileContains -RelativePath 'docs\scripts.md' `
        -Pattern 'CHANGELOG\.md' `
        -Description 'script document must describe changelog structure checks'
}

function Test-GitHubTemplates {
    Test-FileContains -RelativePath '.github\pull_request_template.md' `
        -Pattern 'check-repo\.ps1' `
        -Description 'pull request template must require repository checks'

    Test-FileContains -RelativePath '.github\pull_request_template.md' `
        -Pattern 'examples/examples\.json' `
        -Description 'pull request template must cover example manifest updates'

    Test-FileContains -RelativePath '.github\ISSUE_TEMPLATE\bug_report.md' `
        -Pattern 'g_freertosFaultCode' `
        -Description 'bug report template must request FreeRTOS fault evidence'

    Test-FileContains -RelativePath '.github\ISSUE_TEMPLATE\example_request.md' `
        -Pattern 'examples/examples\.json' `
        -Description 'example request template must mention manifest updates'

    Test-FileContains -RelativePath '.github\ISSUE_TEMPLATE\hardware_validation.md' `
        -Pattern 'docs/hardware-validation-record\.md' `
        -Description 'hardware validation template must link validation record template'
}

function Test-GitHubWorkflow {
    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'runs-on:\s+windows-latest' `
        -Description 'repository check workflow must run on Windows'

    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'actions/checkout@v4' `
        -Description 'repository check workflow must checkout sources'

    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'fetch-depth:\s+0' `
        -Description 'repository check workflow must fetch history for commit-range checks'

    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'check-repo\.ps1' `
        -Description 'repository check workflow must run check-repo.ps1'

    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'git diff --check' `
        -Description 'repository check workflow must check committed whitespace ranges'

    Test-FileContains -RelativePath '.github\workflows\repo-check.yml' `
        -Pattern 'git diff-tree --check' `
        -Description 'repository check workflow must check root commit whitespace'
}

function Test-ValidationStatusDocument {
    $validationDocRelative = 'docs\validation-status.md'

    try {
        $validationDocPath = Resolve-RepoPath -RelativePath $validationDocRelative
    } catch {
        Add-Failure "$validationDocRelative is missing"
        return
    }

    $validationDocText = Get-RepoText -Path $validationDocPath
    $manifestPath = Resolve-RepoPath -RelativePath 'examples\examples.json'
    $manifest = Get-RepoText -Path $manifestPath | ConvertFrom-Json

    foreach ($entry in $manifest.examples) {
        $name = [string]$entry.name
        $status = [string]$entry.validationStatus

        if (-not $status) {
            continue
        }

        $matrixRowPattern = "(?m)^\|\s*``$([regex]::Escape($name))``\s*\|\s*``$([regex]::Escape($status))``\s*\|"
        if ($validationDocText -notmatch $matrixRowPattern) {
            Add-Failure "validation status document must contain matrix row for example '$name' with status '$status'"
        }
    }
}

function Test-HardwareValidationRecordDocument {
    Test-FileContains -RelativePath 'docs\hardware-validation-record.md' `
        -Pattern 'hardware-verified' `
        -Description 'hardware validation record must describe validation status upgrade rule'

    Test-FileContains -RelativePath 'docs\hardware-validation-record.md' `
        -Pattern 'gpio_blink_mdk' `
        -Description 'hardware validation record must cover gpio blink example'

    Test-FileContains -RelativePath 'docs\hardware-validation-record.md' `
        -Pattern 'freertos_signal_adc_uart_mdk' `
        -Description 'hardware validation record must cover integrated demo'

    Test-FileContains -RelativePath 'docs\hardware-validation-record.md' `
        -Pattern 'g_freertosFaultCode' `
        -Description 'hardware validation record must require fault code evidence'

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/hardware-validation-record\.md' `
        -Description 'README must link hardware validation record template'

    Test-FileContains -RelativePath 'docs\hardware-validation.md' `
        -Pattern 'hardware-validation-record\.md' `
        -Description 'hardware validation guide must link record template'

    Test-FileContains -RelativePath 'docs\validation-status.md' `
        -Pattern 'hardware-validation-record\.md' `
        -Description 'validation status document must link hardware validation record template'
}

Write-Host 'Checking tracked file hygiene...'
$trackedFiles = Invoke-Git -Arguments @('ls-files')
foreach ($file in $trackedFiles) {
    Test-BlockedTrackedFile -Path $file
    Test-OwnedTextFileFormat -Path $file
}

Write-Host 'Checking .gitignore policy...'
Test-GitignorePolicy

Write-Host 'Checking .gitattributes policy...'
Test-GitattributesPolicy

$ignoredTrackedFiles = Invoke-Git -Arguments @('ls-files', '-ci', '--exclude-standard')
foreach ($file in $ignoredTrackedFiles) {
    Add-Failure "${file}: tracked file also matches .gitignore"
}

Write-Host 'Checking example manifest...'
Test-ExampleManifest

Write-Host 'Checking third-party provenance...'
Test-ThirdPartyProvenance

Write-Host 'Checking validation status document...'
Test-ValidationStatusDocument

Write-Host 'Checking hardware validation record template...'
Test-HardwareValidationRecordDocument

Write-Host 'Checking known limitations document...'
Test-KnownLimitationsDocument

Write-Host 'Checking new example checklist...'
Test-NewExampleChecklistDocument

Write-Host 'Checking architecture documentation...'
Test-ArchitectureDocument

Write-Host 'Checking script documentation...'
Test-ScriptsDocument

Write-Host 'Checking changelog documentation...'
Test-ChangelogDocument

Write-Host 'Checking GitHub templates...'
Test-GitHubTemplates

Write-Host 'Checking GitHub workflow...'
Test-GitHubWorkflow

if ($Failures.Count -gt 0) {
    Write-Host "Repository check failed with $($Failures.Count) issue(s):"
    foreach ($failure in $Failures) {
        Write-Host " - $failure"
    }
    exit 1
}

Write-Host 'Repository checks passed.'
