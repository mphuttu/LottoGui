[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Release',

    [ValidateSet('Win32', 'x64')]
    [string]$Platform = 'x64',

    [switch]$SkipInstaller
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Get-VisualStudioMsBuild {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

    if (-not (Test-Path $vswhere)) {
        throw "vswhere.exe was not found. Install Visual Studio with the required C++ desktop tools and try again."
    }

    $installationPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.ComponentGroup.NativeDesktop.Core -property installationPath 2>$null
    if (-not $installationPath) {
        throw "Visual Studio installation with C++ desktop tools was not found."
    }

    $candidatePaths = @(
        (Join-Path $installationPath 'MSBuild\Current\Bin\MSBuild.exe'),
        (Join-Path $installationPath 'MSBuild\15.0\Bin\MSBuild.exe'),
        (Join-Path $installationPath 'MSBuild\17.0\Bin\MSBuild.exe')
    )

    foreach ($path in $candidatePaths) {
        if (Test-Path $path) {
            return $path
        }
    }

    throw "MSBuild.exe was not found under the detected Visual Studio installation."
}

function Get-InnoSetupCompiler {
    $candidatePaths = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'),
        (Join-Path ${env:ProgramFiles} 'Inno Setup 6\ISCC.exe')
    )

    foreach ($path in $candidatePaths) {
        if (Test-Path $path) {
            return $path
        }
    }

    throw "Inno Setup Compiler (ISCC.exe) was not found. Install Inno Setup and try again."
}

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$solutionPath = Join-Path $repoRoot 'LottoGui.sln'
$installerScriptPath = Join-Path $repoRoot 'installer\LottoGuiSetup.iss'

if (-not $SkipInstaller -and ($Configuration -ne 'Release' -or $Platform -ne 'x64')) {
    throw "The installer packages only Release|x64. Use -SkipInstaller for other configurations."
}

if (-not (Test-Path $solutionPath)) {
    throw "Solution file not found: $solutionPath"
}

if (-not (Test-Path $installerScriptPath)) {
    throw "Installer script not found: $installerScriptPath"
}

$msbuildPath = Get-VisualStudioMsBuild

Write-Host "Building solution: $solutionPath" -ForegroundColor Cyan
Write-Host "Configuration: $Configuration | Platform: $Platform" -ForegroundColor Cyan

& $msbuildPath $solutionPath /restore /m /p:Configuration=$Configuration /p:Platform=$Platform
if ($LASTEXITCODE -ne 0) {
    throw "The project build failed."
}

if (-not $SkipInstaller) {
    $isccPath = Get-InnoSetupCompiler
    Write-Host "Creating installer with Inno Setup: $installerScriptPath" -ForegroundColor Cyan
    & $isccPath $installerScriptPath
    if ($LASTEXITCODE -ne 0) {
        throw "Inno Setup installer build failed."
    }

    $installerOutput = Join-Path $repoRoot 'installer\Output\LottoGuiSetup_x64.exe'
    if (-not (Test-Path $installerOutput)) {
        throw "Installer compilation succeeded but the expected output was not found: $installerOutput"
    }
    Write-Host "Installer created: $installerOutput" -ForegroundColor Green
}

Write-Host "Build finished successfully." -ForegroundColor Green
