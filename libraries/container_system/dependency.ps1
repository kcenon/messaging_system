#Requires -Version 5.1

<#
.SYNOPSIS
    Container System Dependency Installation Script for Windows (PowerShell)

.DESCRIPTION
    This script installs all required dependencies for the Container System module
    including vcpkg, CMake, Visual Studio Build Tools, and C++ dependencies.

.PARAMETER NoVcpkg
    Skip vcpkg installation

.PARAMETER NoCMake
    Skip CMake installation

.PARAMETER InstallVS
    Install Visual Studio Build Tools automatically

.PARAMETER Force
    Force reinstallation of components

.PARAMETER Help
    Show this help message

.EXAMPLE
    .\dependency.ps1
    Install all dependencies with default settings

.EXAMPLE
    .\dependency.ps1 -InstallVS -Force
    Install all dependencies including Visual Studio Build Tools, forcing reinstallation
#>

param(
    [switch]$NoVcpkg,
    [switch]$NoCMake,
    [switch]$InstallVS,
    [switch]$Force,
    [switch]$Help
)

# Error handling
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $ScriptDir

# Color output functions
function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Green
}

function Write-Error-Custom {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Write-Warning-Custom {
    param([string]$Message)
    Write-Host "[WARNING] $Message" -ForegroundColor Yellow
}

function Write-Step {
    param([string]$Message)
    Write-Host "[STEP] $Message" -ForegroundColor Blue
}

function Show-Help {
    Get-Help $MyInvocation.MyCommand.Definition -Detailed
    exit 0
}

function Test-AdminRights {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-Git {
    try {
        $null = Get-Command git -ErrorAction Stop
        Write-Info "Git is available"
        return $true
    }
    catch {
        Write-Error-Custom "Git is not installed or not in PATH"
        Write-Info "Please install Git for Windows from: https://git-scm.com/download/win"
        return $false
    }
}

function Test-VisualStudio {
    $vsPaths = @(
        "C:\Program Files\Microsoft Visual Studio\2022",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022",
        "C:\Program Files\Microsoft Visual Studio\2019",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019",
        "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools",
        "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools"
    )

    foreach ($path in $vsPaths) {
        if (Test-Path $path) {
            Write-Info "Visual Studio or Build Tools found at: $path"
            return $true
        }
    }

    Write-Warning-Custom "Visual Studio or Build Tools not found"
    return $false
}

function Install-VisualStudio {
    Write-Step "Installing Visual Studio Build Tools..."

    $installerPath = "vs_buildtools.exe"
    $downloadUrl = "https://aka.ms/vs/17/release/vs_buildtools.exe"

    try {
        Write-Info "Downloading Visual Studio Build Tools installer..."
        Invoke-WebRequest -Uri $downloadUrl -OutFile $installerPath -UseBasicParsing

        if (-not (Test-Path $installerPath)) {
            throw "Failed to download installer"
        }

        Write-Info "Installing Visual Studio Build Tools with C++ workload..."
        Write-Warning-Custom "This may take several minutes..."

        $arguments = @(
            "--quiet",
            "--wait",
            "--add", "Microsoft.VisualStudio.Workload.VCTools",
            "--add", "Microsoft.VisualStudio.Component.Windows10SDK.19041"
        )

        $process = Start-Process -FilePath $installerPath -ArgumentList $arguments -Wait -PassThru

        if ($process.ExitCode -ne 0) {
            throw "Visual Studio Build Tools installation failed with exit code: $($process.ExitCode)"
        }

        Write-Info "Visual Studio Build Tools installed successfully"
        return $true
    }
    catch {
        Write-Error-Custom "Visual Studio Build Tools installation failed: $_"
        return $false
    }
    finally {
        if (Test-Path $installerPath) {
            Remove-Item $installerPath -Force
        }
    }
}

function Test-CMake {
    try {
        $null = Get-Command cmake -ErrorAction Stop
        Write-Info "CMake is available"
        return $true
    }
    catch {
        return $false
    }
}

function Install-CMake {
    Write-Step "Installing CMake..."

    $isAdmin = Test-AdminRights
    $cmakeVersion = "3.28.1"
    $downloadUrl = "https://github.com/Kitware/CMake/releases/download/v$cmakeVersion/cmake-$cmakeVersion-windows-x86_64.msi"
    $installerPath = "cmake-installer.msi"

    try {
        Write-Info "Downloading CMake $cmakeVersion..."
        Invoke-WebRequest -Uri $downloadUrl -OutFile $installerPath -UseBasicParsing

        if (-not (Test-Path $installerPath)) {
            throw "Failed to download CMake installer"
        }

        Write-Info "Installing CMake..."

        $arguments = @("/i", $installerPath, "/quiet", "ADD_CMAKE_TO_PATH=System")

        if (-not $isAdmin) {
            Write-Warning-Custom "Installing CMake for current user (no admin rights)"
            $arguments += "ALLUSERS=0"
        }

        $process = Start-Process -FilePath "msiexec.exe" -ArgumentList $arguments -Wait -PassThru

        if ($process.ExitCode -ne 0) {
            throw "CMake installation failed with exit code: $($process.ExitCode)"
        }

        Write-Info "CMake installed successfully"

        # Refresh environment variables
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")

        return $true
    }
    catch {
        Write-Error-Custom "CMake installation failed: $_"
        return $false
    }
    finally {
        if (Test-Path $installerPath) {
            Remove-Item $installerPath -Force
        }
    }
}

function Test-Vcpkg {
    # Check if vcpkg is in PATH
    try {
        $null = Get-Command vcpkg -ErrorAction Stop
        Write-Info "vcpkg is available in PATH"
        return $true
    }
    catch {
        # Check common locations
        $vcpkgPaths = @(
            "C:\vcpkg",
            "C:\tools\vcpkg",
            "C:\dev\vcpkg",
            "$env:USERPROFILE\vcpkg"
        )

        foreach ($path in $vcpkgPaths) {
            $vcpkgExe = Join-Path $path "vcpkg.exe"
            if (Test-Path $vcpkgExe) {
                Write-Info "Found vcpkg at: $path"
                $env:Path = "$path;$env:Path"
                $global:VcpkgRoot = $path
                return $true
            }
        }

        return $false
    }
}

function Install-Vcpkg {
    Write-Step "Installing vcpkg..."

    $vcpkgRoot = "$env:USERPROFILE\vcpkg"
    $global:VcpkgRoot = $vcpkgRoot

    if (Test-Path $vcpkgRoot) {
        if ($Force) {
            Write-Info "Removing existing vcpkg installation..."
            Remove-Item $vcpkgRoot -Recurse -Force
        }
        else {
            Write-Info "vcpkg directory already exists at: $vcpkgRoot"
            Write-Info "Use -Force to reinstall"
            return $false
        }
    }

    try {
        Write-Info "Cloning vcpkg to: $vcpkgRoot"
        & git clone https://github.com/Microsoft/vcpkg.git $vcpkgRoot

        if ($LASTEXITCODE -ne 0) {
            throw "Failed to clone vcpkg repository"
        }

        Push-Location $vcpkgRoot

        Write-Info "Bootstrapping vcpkg..."
        & .\bootstrap-vcpkg.bat

        if ($LASTEXITCODE -ne 0) {
            throw "Failed to bootstrap vcpkg"
        }

        # Add to PATH for current session
        $env:Path = "$vcpkgRoot;$env:Path"

        Write-Info "vcpkg installed successfully at: $vcpkgRoot"
        Write-Warning-Custom "Add $vcpkgRoot to your system PATH for permanent access"

        return $true
    }
    catch {
        Write-Error-Custom "vcpkg installation failed: $_"
        return $false
    }
    finally {
        Pop-Location
    }
}

function Install-CppDependencies {
    Write-Step "Installing C++ dependencies..."

    # Create vcpkg.json if it doesn't exist
    if (-not (Test-Path "vcpkg.json")) {
        Write-Warning-Custom "vcpkg.json not found. Creating default configuration..."

        $vcpkgConfig = @{
            name = "container-system"
            description = "Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration"
            dependencies = @("fmt", "gtest", "benchmark")
        } | ConvertTo-Json -Depth 3

        Set-Content -Path "vcpkg.json" -Value $vcpkgConfig -Encoding UTF8
    }

    try {
        Write-Info "Installing vcpkg dependencies..."
        & vcpkg install --triplet x64-windows

        if ($LASTEXITCODE -ne 0) {
            throw "Failed to install vcpkg dependencies"
        }

        Write-Info "C++ dependencies installed successfully"
        return $true
    }
    catch {
        Write-Error-Custom "Failed to install C++ dependencies: $_"
        return $false
    }
}

function Install-TestFramework {
    Write-Step "Checking test frameworks..."

    try {
        # Check if already installed via vcpkg
        $vcpkgList = & vcpkg list 2>$null

        if ($vcpkgList -match "gtest") {
            Write-Info "Google Test is already installed via vcpkg"
        }
        else {
            Write-Warning-Custom "Installing Google Test..."
            & vcpkg install gtest --triplet x64-windows
        }

        if ($vcpkgList -match "benchmark") {
            Write-Info "Google Benchmark is already installed via vcpkg"
        }
        else {
            Write-Warning-Custom "Installing Google Benchmark..."
            & vcpkg install benchmark --triplet x64-windows
        }

        return $true
    }
    catch {
        Write-Error-Custom "Failed to install test frameworks: $_"
        return $false
    }
}

# Main execution
function Main {
    if ($Help) {
        Show-Help
    }

    Write-Info "Container System Dependency Installer for Windows (PowerShell)"
    Write-Info "OS: Windows $([Environment]::OSVersion.Version)"
    Write-Info "PowerShell: $($PSVersionTable.PSVersion)"
    ""

    # Check Git
    if (-not (Test-Git)) {
        exit 1
    }

    # Check Visual Studio
    if (-not (Test-VisualStudio)) {
        if ($InstallVS) {
            if (-not (Install-VisualStudio)) {
                exit 1
            }
        }
        else {
            Write-Error-Custom "Visual Studio or Build Tools required"
            Write-Info "Install Visual Studio 2019 or later with C++ workload"
            Write-Info "Or use -InstallVS to install Build Tools automatically"
            exit 1
        }
    }

    # Install CMake if needed
    if (-not $NoCMake) {
        if (-not (Test-CMake)) {
            if (-not (Install-CMake)) {
                exit 1
            }
        }
    }

    # Install vcpkg if needed
    if (-not $NoVcpkg) {
        if (-not (Test-Vcpkg)) {
            if (-not (Install-Vcpkg)) {
                exit 1
            }
        }
    }

    # Install C++ dependencies
    if (-not (Install-CppDependencies)) {
        exit 1
    }

    # Install test frameworks
    if (-not (Install-TestFramework)) {
        # Non-fatal
    }

    ""
    Write-Info "All dependencies installed successfully!"
    Write-Info "You can now build the project with build.bat or build.ps1"
    ""
    Write-Step "For CMake integration, use:"

    if ($global:VcpkgRoot) {
        Write-Info "cmake -DCMAKE_TOOLCHAIN_FILE=$global:VcpkgRoot\scripts\buildsystems\vcpkg.cmake .."
        Write-Info "Or set VCPKG_ROOT environment variable to: $global:VcpkgRoot"
    }
    else {
        Write-Info "cmake -DCMAKE_TOOLCHAIN_FILE=<vcpkg-root>\scripts\buildsystems\vcpkg.cmake .."
    }
}

# Run main function
Main