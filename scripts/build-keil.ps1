param(
    [string]$UV4Path,
    [string[]]$ExampleName,
    [ValidateSet('Build', 'Rebuild')]
    [string]$Mode = 'Rebuild',
    [switch]$ListExamples,
    [switch]$CleanOnly,
    [switch]$CleanAfterBuild
)

$ErrorActionPreference = 'Stop'

$RepoRoot = (Resolve-Path -LiteralPath (Split-Path -Parent $PSScriptRoot)).Path
$ExampleManifestPath = Join-Path -Path $RepoRoot -ChildPath 'examples\examples.json'

function Assert-ParameterCombination {
    param([hashtable]$BoundParameters)

    $errors = @()

    if ($BoundParameters.ContainsKey('ListExamples')) {
        $conflicts = @('ExampleName', 'UV4Path', 'Mode', 'CleanOnly', 'CleanAfterBuild') |
            Where-Object { $BoundParameters.ContainsKey($_) }
        if ($conflicts.Count -gt 0) {
            $errors += "-ListExamples cannot be combined with -$($conflicts -join ', -')."
        }
    }

    if ($BoundParameters.ContainsKey('CleanOnly')) {
        $conflicts = @('UV4Path', 'Mode', 'CleanAfterBuild') |
            Where-Object { $BoundParameters.ContainsKey($_) }
        if ($conflicts.Count -gt 0) {
            $errors += "-CleanOnly cannot be combined with -$($conflicts -join ', -')."
        }
    }

    if ($errors.Count -gt 0) {
        throw "Invalid parameter combination:`r`n- $($errors -join "`r`n- ")"
    }
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

function Get-ExampleProjects {
    if (-not (Test-Path -LiteralPath $ExampleManifestPath)) {
        throw "Example manifest not found: $ExampleManifestPath"
    }

    $manifest = Get-Content -LiteralPath $ExampleManifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
    if (-not $manifest.examples) {
        throw "Example manifest has no examples: $ExampleManifestPath"
    }

    $projects = @()
    foreach ($entry in $manifest.examples) {
        if (-not $entry.name -or -not $entry.project) {
            throw 'Each example manifest entry must define name and project.'
        }

        $target = if ($entry.target) { [string]$entry.target } else { 'Example' }
        $projects += [pscustomobject]@{
            Name = [string]$entry.name
            Description = if ($entry.description) { [string]$entry.description } else { '' }
            ProjectRelative = [string]$entry.project
            Project = Resolve-RepoPath -RelativePath ([string]$entry.project)
            Target = $target
            ValidationStatus = if ($entry.validationStatus) { [string]$entry.validationStatus } else { '' }
        }
    }

    return $projects
}

function Select-ExampleProjects {
    param(
        [array]$Projects,
        [string[]]$Names
    )

    if (-not $Names -or ($Names.Count -eq 0)) {
        return $Projects
    }

    $selected = @()
    foreach ($name in $Names) {
        $match = @($Projects | Where-Object { $_.Name -ieq $name })
        if ($match.Count -eq 0) {
            $available = ($Projects | ForEach-Object { $_.Name }) -join ', '
            throw "Unknown example '$name'. Available examples: $available"
        }
        $selected += $match[0]
    }

    return $selected
}

function Resolve-Uv4Path {
    param([string]$RequestedPath)

    $candidates = @()

    if ($RequestedPath) {
        $candidates += $RequestedPath
    }

    if ($env:KEIL_UV4) {
        $candidates += $env:KEIL_UV4
    }

    $pathCommand = Get-Command UV4.exe -ErrorAction SilentlyContinue
    if ($pathCommand) {
        $candidates += $pathCommand.Source
    }

    $candidates += @(
        'D:\keil\MDK542a\UV4\UV4.exe',
        'C:\Keil_v5\UV4\UV4.exe',
        'C:\Keil\UV4\UV4.exe',
        'C:\Program Files\Keil_v5\UV4\UV4.exe',
        'C:\Program Files (x86)\Keil_v5\UV4\UV4.exe'
    )

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path -LiteralPath $candidate)) {
            return (Resolve-Path -LiteralPath $candidate).Path
        }
    }

    throw 'UV4.exe not found. Pass -UV4Path or set KEIL_UV4.'
}

function Remove-ItemWithRetry {
    param([string]$Path)

    for ($attempt = 1; $attempt -le 5; $attempt++) {
        try {
            Remove-Item -LiteralPath $Path -Recurse -Force
            return
        } catch {
            if ($attempt -eq 5) {
                throw
            }
            Start-Sleep -Milliseconds 300
        }
    }
}

function Remove-KeilOutputs {
    param([string]$ProjectPath)

    $projectDir = Split-Path -Parent $ProjectPath
    $resolvedProjectDir = (Resolve-Path -LiteralPath $projectDir).Path

    if (-not (Test-PathWithinDirectory -Path $resolvedProjectDir -Directory $RepoRoot)) {
        throw "Refuse to clean outside repository: $resolvedProjectDir"
    }

    $targets = @()
    $targets += (Join-Path -Path $resolvedProjectDir -ChildPath 'Objects')
    $targets += (Join-Path -Path $resolvedProjectDir -ChildPath 'Listings')

    $projectFiles = Get-ChildItem -LiteralPath $resolvedProjectDir -Force -File -ErrorAction SilentlyContinue
    foreach ($file in $projectFiles) {
        $isGeneratedFile = (($file.Name -like '*.uvoptx') -or ($file.Name -like '*.uvguix.*') -or ($file.Name -like '*.uvgui.*') -or ($file.Name -eq 'JLinkLog.txt') -or ($file.Name -eq 'JLinkSettings.ini'))
        if ($isGeneratedFile) {
            $targets += $file.FullName
        }
    }

    foreach ($target in $targets) {
        if (Test-Path -LiteralPath $target) {
            $resolvedTarget = (Resolve-Path -LiteralPath $target).Path
            if (-not (Test-PathWithinDirectory -Path $resolvedTarget -Directory $resolvedProjectDir)) {
                throw "Refuse to clean outside project MDK directory: $resolvedTarget"
            }
            Remove-ItemWithRetry -Path $resolvedTarget
        }
    }
}

function Get-BuildSummary {
    param([string]$ProjectPath)

    $projectDir = Split-Path -Parent $ProjectPath
    $log = Get-ChildItem -LiteralPath (Join-Path $projectDir 'Objects') -Filter '*.build_log.htm' -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1

    if (-not $log) {
        throw "Build log not found for $ProjectPath"
    }

    $content = Get-Content -LiteralPath $log.FullName -Raw
    $summary = [regex]::Match($content, '(\d+)\s+Error\(s\),\s+(\d+)\s+Warning\(s\)')
    if (-not $summary.Success) {
        throw "Could not parse build summary: $($log.FullName)"
    }

    $programSize = [regex]::Match($content, 'Program Size:\s+([^\r\n<]+)')

    [pscustomobject]@{
        Log = $log.FullName
        Errors = [int]$summary.Groups[1].Value
        Warnings = [int]$summary.Groups[2].Value
        ProgramSize = if ($programSize.Success) { $programSize.Groups[1].Value.Trim() } else { '' }
    }
}

Assert-ParameterCombination -BoundParameters $PSBoundParameters

$AllProjects = Get-ExampleProjects

if ($ListExamples) {
    $AllProjects | Select-Object Name, Target, ValidationStatus, ProjectRelative, Description | Format-Table -AutoSize
    exit 0
}

$Projects = Select-ExampleProjects -Projects $AllProjects -Names $ExampleName

foreach ($project in $Projects) {
    if (-not (Test-Path -LiteralPath $project.Project)) {
        throw "Project not found: $($project.Project)"
    }
}

if ($CleanOnly) {
    foreach ($project in $Projects) {
        Remove-KeilOutputs -ProjectPath $project.Project
        Write-Host "cleaned $($project.Name)"
    }
    exit 0
}

$uv4 = Resolve-Uv4Path -RequestedPath $UV4Path
$buildSwitch = if ($Mode -eq 'Rebuild') { '-r' } else { '-b' }

Write-Host "Using UV4: $uv4"

foreach ($project in $Projects) {
    Write-Host "Building $($project.Name) [$Mode]..."
    $argumentLine = "$buildSwitch `"$($project.Project)`" -t `"$($project.Target)`""
    $process = Start-Process -FilePath $uv4 -ArgumentList $argumentLine -Wait -PassThru -WindowStyle Hidden

    if ($process.ExitCode -ne 0) {
        throw "Keil build failed for $($project.Name), exit code $($process.ExitCode)"
    }

    $summary = Get-BuildSummary -ProjectPath $project.Project
    Write-Host "$($project.Name): $($summary.Errors) error(s), $($summary.Warnings) warning(s), $($summary.ProgramSize)"

    if (($summary.Errors -ne 0) -or ($summary.Warnings -ne 0)) {
        throw "Keil build is not clean for $($project.Name). See $($summary.Log)"
    }
}

if ($CleanAfterBuild) {
    foreach ($project in $Projects) {
        Remove-KeilOutputs -ProjectPath $project.Project
    }
    Write-Host 'Cleaned Keil generated outputs.'
}

Write-Host 'All Keil builds passed.'
