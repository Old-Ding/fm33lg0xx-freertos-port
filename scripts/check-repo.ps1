param()

$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$Failures = [System.Collections.Generic.List[string]]::new()

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

function Resolve-RepoPath {
    param([string]$RelativePath)

    $fullPath = Join-Path -Path $RepoRoot -ChildPath $RelativePath
    $resolvedPath = (Resolve-Path -LiteralPath $fullPath).Path

    if (-not $resolvedPath.StartsWith($RepoRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Path escapes repository: $RelativePath"
    }

    return $resolvedPath
}

function Test-BlockedTrackedFile {
    param([string]$Path)

    $normalized = $Path -replace '\\', '/'

    $blockedPathPatterns = @(
        @{ Pattern = '^例程/'; Reason = 'vendor reference examples must stay local' },
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

function Test-ExampleManifest {
    $manifestPath = Join-Path -Path $RepoRoot -ChildPath 'examples\examples.json'

    if (-not (Test-Path -LiteralPath $manifestPath)) {
        Add-Failure 'examples/examples.json is missing'
        return
    }

    try {
        $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    } catch {
        Add-Failure "examples/examples.json is not valid JSON: $($_.Exception.Message)"
        return
    }

    if (-not $manifest.examples) {
        Add-Failure 'examples/examples.json must define examples'
        return
    }

    $names = @{}
    foreach ($entry in $manifest.examples) {
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

        if (-not $entry.project) {
            Add-Failure "example '$name' is missing project"
            continue
        }

        $projectRelative = [string]$entry.project
        if ($projectRelative -notmatch '\.uvprojx$') {
            Add-Failure "example '$name' project must be a .uvprojx file: $projectRelative"
        }

        try {
            $projectPath = Resolve-RepoPath -RelativePath $projectRelative
        } catch {
            Add-Failure "example '$name' project path is invalid: $projectRelative"
            continue
        }

        $projectText = Get-Content -LiteralPath $projectPath -Raw
        if ($projectText -notmatch 'FreeRTOS-Kernel-main') {
            Add-Failure "example '$name' must reference the shared FreeRTOS-Kernel-main"
        }

        $projectDir = Split-Path -Parent $projectPath
        $exampleRoot = Split-Path -Parent $projectDir
        $configPath = Join-Path -Path $exampleRoot -ChildPath 'Inc\FreeRTOSConfig.h'

        if (Test-Path -LiteralPath $configPath) {
            $configText = Get-Content -LiteralPath $configPath -Raw
            # 信号量依赖 queue.c；这里检查工程引用，避免新增 API 后只改配置不改 Keil 工程。
            if (($configText -match '#define\s+configUSE_COUNTING_SEMAPHORES\s+1') -and
                ($projectText -notmatch 'queue\.c')) {
                Add-Failure "example '$name' enables counting semaphores but project does not reference queue.c"
            }
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

    $content = Get-Content -LiteralPath $path -Raw
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

    $exampleRoots = @(
        'gpio_blink_mdk',
        'examples\freertos_signal_adc_uart_mdk'
    )
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

Write-Host 'Checking tracked file hygiene...'
$trackedFiles = Invoke-Git -Arguments @('ls-files')
foreach ($file in $trackedFiles) {
    Test-BlockedTrackedFile -Path $file
}

$ignoredTrackedFiles = Invoke-Git -Arguments @('ls-files', '-ci', '--exclude-standard')
foreach ($file in $ignoredTrackedFiles) {
    Add-Failure "${file}: tracked file also matches .gitignore"
}

Write-Host 'Checking example manifest...'
Test-ExampleManifest

Write-Host 'Checking third-party provenance...'
Test-ThirdPartyProvenance

if ($Failures.Count -gt 0) {
    Write-Error "Repository check failed with $($Failures.Count) issue(s):"
    foreach ($failure in $Failures) {
        Write-Error " - $failure"
    }
    exit 1
}

Write-Host 'Repository checks passed.'
