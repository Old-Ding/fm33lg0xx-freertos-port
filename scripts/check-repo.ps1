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
        '*.hex',
        '*.map',
        '*.crf',
        '*.o',
        '*.d',
        '*.pack',
        '*.zip'
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

        $projectText = Get-RepoText -Path $projectPath
        if ($projectText -notmatch 'FreeRTOS-Kernel-main') {
            Add-Failure "example '$name' must reference the shared FreeRTOS-Kernel-main"
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

    Test-FileContains -RelativePath 'README.md' `
        -Pattern 'docs/scripts\.md' `
        -Description 'README must link script document'
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

    Test-FileContains -RelativePath '.github\ISSUE_TEMPLATE\hardware_validation.md' `
        -Pattern 'docs/hardware-validation-record\.md' `
        -Description 'hardware validation issue template must link record template'
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

if ($Failures.Count -gt 0) {
    Write-Host "Repository check failed with $($Failures.Count) issue(s):"
    foreach ($failure in $Failures) {
        Write-Host " - $failure"
    }
    exit 1
}

Write-Host 'Repository checks passed.'
