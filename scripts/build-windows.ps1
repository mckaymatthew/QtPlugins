<#
.SYNOPSIS
    Builds QtPlugins on Windows x64 with MSVC 2019 (v142) and Ninja.

.DESCRIPTION
    Finds a Visual Studio installation with the MSVC 2019 toolset (VS 2019, or VS 2022 with the
    "MSVC v142 - VS 2019 C++ x64/x86 build tools" component), loads its x64 environment, checks
    the compiler, then configures and builds. Optionally runs the tests, deploys a self-contained
    folder with windeployqt, and launches the host.

    Works in Windows PowerShell 5.1 and PowerShell 7. If scripts are blocked, run it as:
        powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1 -Test

.PARAMETER QtDir
    Qt kit directory, e.g. C:\Qt\6.5.3\msvc2019_64. Defaults to $env:QT_ROOT_DIR, then the
    usual install locations.

.PARAMETER Config
    Release (default) or Debug.

.PARAMETER BuildDir
    Build directory, relative to the repository root. Default: build.

.PARAMETER Clean
    Delete the build directory first.

.PARAMETER Test
    Run ctest after building.

.PARAMETER Deploy
    Install into -DeployDir and run windeployqt, giving a folder that runs without Qt.

.PARAMETER DeployDir
    Deployment directory, relative to the repository root. Default: dist.

.PARAMETER Run
    Launch the host afterwards, with an extension open (see -Open).

.PARAMETER Open
    Extension id to open with -Run. Default: org.qtplugins.mustardmayo.

.PARAMETER Software
    Render the tests and -Run with Direct3D 11's built-in software rasterizer (WARP) instead of the
    GPU. Use it in VMs or on machines whose graphics driver is missing or broken. Nothing extra
    needs installing: WARP ships with Windows.

.EXAMPLE
    scripts\build-windows.ps1 -Test
.EXAMPLE
    scripts\build-windows.ps1 -QtDir D:\Qt\6.5.3\msvc2019_64 -Config Debug -Run
.EXAMPLE
    scripts\build-windows.ps1 -Clean -Test -Deploy
.EXAMPLE
    scripts\build-windows.ps1 -Test -Run -Software
#>
[CmdletBinding()]
param(
    [string]$QtDir,
    [ValidateSet('Release', 'Debug')]
    [string]$Config = 'Release',
    [string]$BuildDir = 'build',
    [switch]$Clean,
    [switch]$Test,
    [switch]$Deploy,
    [string]$DeployDir = 'dist',
    [switch]$Run,
    [string]$Open = 'org.qtplugins.mustardmayo',
    [switch]$Software
)

$ErrorActionPreference = 'Stop'
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Toolset = '14.29' # MSVC 2019 (v142), matching Qt's msvc2019_64 kit

function Write-Step([string]$Message) {
    Write-Host "==> $Message" -ForegroundColor Cyan
}

# Runs a native command and stops the script if it fails.
function Invoke-Native {
    param([string]$Command, [string[]]$Arguments)
    & $Command @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Command failed with exit code $LASTEXITCODE"
    }
}

function Find-QtDir {
    $candidates = @()
    if ($QtDir) { $candidates += $QtDir }
    if ($env:QT_ROOT_DIR) { $candidates += $env:QT_ROOT_DIR }
    foreach ($root in @('C:\Qt', "$env:USERPROFILE\Qt")) {
        foreach ($version in @('6.5.3', '6.4.3', '6.4.2')) {
            $candidates += Join-Path $root "$version\msvc2019_64"
        }
    }
    foreach ($dir in $candidates) {
        if (Test-Path (Join-Path $dir 'lib\cmake\Qt6\Qt6Config.cmake')) {
            return (Resolve-Path $dir).Path
        }
    }
    throw "Qt not found. Pass -QtDir <path to the msvc2019_64 kit>, e.g. C:\Qt\6.5.3\msvc2019_64. Tried: $($candidates -join '; ')"
}

function Get-CompilerBanner {
    # cl prints its banner on stderr; capture it through cmd so PowerShell doesn't treat it as an error.
    return (cmd.exe /c 'cl 2>&1' | Out-String)
}

function Test-Msvc2019x64 {
    if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) { return $false }
    $banner = Get-CompilerBanner
    return ($banner -match 'Version 19\.29\.' -and $banner -match 'for x64')
}

function Import-Msvc2019Environment {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path $vswhere)) {
        throw "Visual Studio not found (no vswhere.exe). Install VS 2019, or VS 2022 with the MSVC v142 build tools."
    }
    $installs = & $vswhere -all -products * -property installationPath
    foreach ($install in $installs) {
        $vcvars = Join-Path $install 'VC\Auxiliary\Build\vcvars64.bat'
        $tools = Get-ChildItem -Path (Join-Path $install 'VC\Tools\MSVC') -Filter "$Toolset.*" -Directory -ErrorAction SilentlyContinue
        if ((Test-Path $vcvars) -and $tools) {
            Write-Step "Loading MSVC $Toolset x64 environment from $install"
            $output = cmd.exe /c "`"$vcvars`" -vcvars_ver=$Toolset >nul && set"
            if ($LASTEXITCODE -ne 0) { throw "vcvars64.bat failed" }
            foreach ($line in $output) {
                if ($line -match '^([^=]+)=(.*)$') {
                    Set-Item -Path "Env:$($Matches[1])" -Value $Matches[2]
                }
            }
            return
        }
    }
    throw "No Visual Studio installation has the MSVC 2019 (v142, $Toolset) toolset. Install VS 2019, or add 'MSVC v142 - VS 2019 C++ x64/x86 build tools' to VS 2022."
}

# --- Toolchain -----------------------------------------------------------------------------------

if (Test-Msvc2019x64) {
    Write-Step 'Using the MSVC 2019 x64 environment already loaded'
} else {
    Import-Msvc2019Environment
    if (-not (Test-Msvc2019x64)) {
        throw "Expected MSVC 2019 (19.29) for x64 after loading the environment, got:`n$(Get-CompilerBanner)"
    }
}
Write-Host ((Get-CompilerBanner) -split "`n")[0]

foreach ($tool in @('cmake', 'ninja')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool not found. It ships with Visual Studio's C++ workload, or install it separately."
    }
}

$Qt = Find-QtDir
Write-Step "Using Qt from $Qt"

# --- Build ---------------------------------------------------------------------------------------

$Build = Join-Path $RepoRoot $BuildDir
if ($Clean -and (Test-Path $Build)) {
    Write-Step "Removing $Build"
    Remove-Item -Recurse -Force $Build
}

Write-Step "Configuring ($Config)"
Invoke-Native cmake @('-S', $RepoRoot, '-B', $Build, '-G', 'Ninja',
                      "-DCMAKE_BUILD_TYPE=$Config", "-DCMAKE_PREFIX_PATH=$Qt")

Write-Step 'Building'
Invoke-Native cmake @('--build', $Build)

# Qt Quick renders through Direct3D 11 on Windows, which is part of the OS. These variables pin
# that backend and make it use WARP, the software rasterizer that ships with Windows.
if ($Software) {
    Write-Step 'Using Direct3D 11 software rendering (WARP)'
    $env:QSG_RHI_BACKEND = 'd3d11'
    $env:QSG_RHI_PREFER_SOFTWARE_RENDERER = '1'
}

if ($Test) {
    Write-Step 'Testing'
    Invoke-Native ctest @('--test-dir', $Build, '--output-on-failure')
}

if ($Deploy) {
    $Dist = Join-Path $RepoRoot $DeployDir
    Write-Step "Deploying to $Dist"
    Invoke-Native cmake @('--install', $Build, '--prefix', $Dist)
    Write-Host "Self-contained build: $(Join-Path $Dist 'bin\qtplugins-host.exe')"
}

if ($Run) {
    Write-Step "Running the host with $Open"
    $env:PATH = "$(Join-Path $Qt 'bin');$env:PATH"
    Start-Process -FilePath (Join-Path $Build 'host\qtplugins-host.exe') `
                  -ArgumentList @('--extensions-dir', (Join-Path $Build 'extensions'), '--open', $Open)
}

Write-Step 'Done'
