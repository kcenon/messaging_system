#Requires -Version 5.1

<#
.SYNOPSIS
    Container System Build Script for Windows (PowerShell)

.DESCRIPTION
    This script builds the container system library with all features, tests, and examples
    using CMake and Visual Studio or Ninja build system.

.PARAMETER Debug
    Build in Debug mode (default: Release)

.PARAMETER Clean
    Clean build (remove build directory first)

.PARAMETER NoTests
    Don't build tests

.PARAMETER NoExamples
    Don't build examples

.PARAMETER NoSamples
    Don't build samples

.PARAMETER Docs
    Build documentation

.PARAMETER NoMessaging
    Disable messaging features

.PARAMETER NoMetrics
    Disable performance metrics

.PARAMETER NoIntegration
    Disable external integration

.PARAMETER Verbose
    Verbose output

.PARAMETER Jobs
    Number of parallel jobs (default: number of processors)

.PARAMETER X86
    Build for x86 (default: x64)

.PARAMETER VS2019
    Use Visual Studio 2019

.PARAMETER VS2022
    Use Visual Studio 2022 (default)

.PARAMETER Ninja
    Use Ninja generator

.PARAMETER Help
    Show this help message

.EXAMPLE
    .\build.ps1
    Build with default settings (Release mode, all features enabled)

.EXAMPLE
    .\build.ps1 -Debug -Verbose
    Build in Debug mode with verbose output

.EXAMPLE
    .\build.ps1 -Clean -NoTests -NoExamples
    Clean build without tests or examples
#>

param(
    [switch]$Debug,
    [switch]$Clean,
    [switch]$NoTests,
    [switch]$NoExamples,
    [switch]$NoSamples,
    [switch]$Docs,
    [switch]$NoMessaging,
    [switch]$NoMetrics,
    [switch]$NoIntegration,
    [switch]$Verbose,
    [int]$Jobs = $env:NUMBER_OF_PROCESSORS,
    [switch]$X86,
    [switch]$VS2019,
    [switch]$VS2022,
    [switch]$Ninja,
    [switch]$Help
)

# Error handling
$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

# Script directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Set-Location $ScriptDir

# Configuration
$BuildType = if ($Debug) { "Debug" } else { "Release" }
$BuildTests = if ($NoTests) { "OFF" } else { "ON" }
$BuildExamples = if ($NoExamples) { "OFF" } else { "ON" }
$BuildSamples = if ($NoSamples) { "OFF" } else { "ON" }
$BuildDocs = if ($Docs) { "ON" } else { "OFF" }
$MessagingFeatures = if ($NoMessaging) { "OFF" } else { "ON" }
$PerformanceMetrics = if ($NoMetrics) { "OFF" } else { "ON" }
$ExternalIntegration = if ($NoIntegration) { "OFF" } else { "ON" }
$Platform = if ($X86) { "Win32" } else { "x64" }

# Determine generator
$Generator = "Visual Studio 17 2022"
if ($VS2019) { $Generator = "Visual Studio 16 2019" }
if ($VS2022) { $Generator = "Visual Studio 17 2022" }
if ($Ninja) { $Generator = "Ninja" }

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

function Test-CMake {
    try {
        $null = Get-Command cmake -ErrorAction Stop
        return $true
    }
    catch {
        Write-Error-Custom "CMake not found. Please install CMake or run dependency.ps1"
        return $false
    }
}

function Test-Vcpkg {
    # Check if vcpkg is in PATH
    try {
        $null = Get-Command vcpkg -ErrorAction Stop
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
                $global:VcpkgRoot = $path
                $env:Path = "$path;$env:Path"
                return $true
            }
        }

        Write-Error-Custom "vcpkg not found. Please install vcpkg or run dependency.ps1"
        return $false
    }
}

function Test-VisualStudio {
    if ($Generator -eq "Ninja") {
        return $true  # Skip VS check for Ninja
    }

    try {
        # Test if the generator is available
        $testResult = & cmake -G $Generator --help 2>$null
        return $LASTEXITCODE -eq 0
    }
    catch {
        Write-Error-Custom "Visual Studio generator '$Generator' not available"
        Write-Info "Please install Visual Studio 2019 or later with C++ workload"
        return $false
    }
}

function Invoke-Build {
    # Print build configuration
    Write-Info "Container System Build Configuration:"
    Write-Info "  Build Type: $BuildType"
    Write-Info "  Platform: $Platform"
    Write-Info "  Generator: $Generator"
    Write-Info "  Build Tests: $BuildTests"
    Write-Info "  Build Examples: $BuildExamples"
    Write-Info "  Build Samples: $BuildSamples"
    Write-Info "  Build Docs: $BuildDocs"
    Write-Info "  Messaging Features: $MessagingFeatures"
    Write-Info "  Performance Metrics: $PerformanceMetrics"
    Write-Info "  External Integration: $ExternalIntegration"
    Write-Info "  Parallel Jobs: $Jobs"
    Write-Info "  Clean Build: $Clean"
    ""

    # Check prerequisites
    if (-not (Test-CMake)) { return $false }
    if (-not (Test-Vcpkg)) { return $false }
    if (-not (Test-VisualStudio)) { return $false }

    # Clean build directory if requested
    if ($Clean) {
        Write-Info "Cleaning build directory..."
        if (Test-Path "build") {
            Remove-Item "build" -Recurse -Force
        }
    }

    # Create build directory
    Write-Info "Creating build directory..."
    if (-not (Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }

    Push-Location "build"

    try {
        # Configure with CMake
        Write-Step "Configuring with CMake..."

        $cmakeArgs = @(
            "-DCMAKE_BUILD_TYPE=$BuildType",
            "-DUSE_UNIT_TEST=$BuildTests",
            "-DBUILD_CONTAINER_EXAMPLES=$BuildExamples",
            "-DBUILD_CONTAINER_SAMPLES=$BuildSamples",
            "-DENABLE_MESSAGING_FEATURES=$MessagingFeatures",
            "-DENABLE_PERFORMANCE_METRICS=$PerformanceMetrics",
            "-DENABLE_EXTERNAL_INTEGRATION=$ExternalIntegration"
        )

        # Add vcpkg toolchain if available
        if ($global:VcpkgRoot) {
            $toolchainFile = Join-Path $global:VcpkgRoot "scripts\buildsystems\vcpkg.cmake"
            $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$toolchainFile"
        }

        if ($Verbose) {
            $cmakeArgs += "-DCMAKE_VERBOSE_MAKEFILE=ON"
        }

        # Set generator and platform
        if ($Generator -eq "Ninja") {
            $cmakeArgs = @("-G", "Ninja") + $cmakeArgs + ".."
        }
        else {
            $cmakeArgs = @("-G", $Generator, "-A", $Platform) + $cmakeArgs + ".."
        }

        & cmake @cmakeArgs

        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed!"
        }

        # Build
        Write-Step "Building..."

        $buildArgs = @("--build", ".", "--config", $BuildType, "--parallel", $Jobs)

        if ($Verbose) {
            $buildArgs += "--verbose"
        }

        & cmake @buildArgs

        if ($LASTEXITCODE -ne 0) {
            throw "Build failed!"
        }

        # Run tests if built
        if ($BuildTests -eq "ON") {
            Write-Step "Running tests..."

            # Run unit tests
            $unitTestExe = Join-Path $BuildType "unit_tests.exe"
            if (Test-Path $unitTestExe) {
                Write-Info "Running unit tests..."
                & $unitTestExe
                if ($LASTEXITCODE -eq 0) {
                    Write-Info "Unit tests passed!"
                }
                else {
                    Write-Error-Custom "Unit tests failed!"
                    return $false
                }
            }

            # Run integration tests
            $integrationTestExe = Join-Path $BuildType "test_messaging_integration.exe"
            if (Test-Path $integrationTestExe) {
                Write-Info "Running integration tests..."
                & $integrationTestExe
                if ($LASTEXITCODE -eq 0) {
                    Write-Info "Integration tests passed!"
                }
                else {
                    Write-Error-Custom "Integration tests failed!"
                    return $false
                }
            }

            # Run performance tests (but don't fail on performance issues)
            $perfTestExe = Join-Path $BuildType "performance_tests.exe"
            if (Test-Path $perfTestExe) {
                Write-Info "Running performance tests..."
                & $perfTestExe
                if ($LASTEXITCODE -eq 0) {
                    Write-Info "Performance tests completed!"
                }
                else {
                    Write-Warning-Custom "Performance tests had issues (this is informational only)"
                }
            }

            # Run all tests via CTest if available
            try {
                $null = Get-Command ctest -ErrorAction Stop
                Write-Info "Running all tests via CTest..."
                & ctest --build-config $BuildType --output-on-failure
                if ($LASTEXITCODE -eq 0) {
                    Write-Info "All CTest tests passed!"
                }
                else {
                    Write-Warning-Custom "Some CTest tests failed"
                }
            }
            catch {
                # CTest not available, skip
            }
        }

        return $true
    }
    catch {
        Write-Error-Custom "Build process failed: $_"
        return $false
    }
    finally {
        Pop-Location
    }
}

function Show-BuildSummary {
    Write-Info "Build completed successfully!"
    Write-Info "Build artifacts are in: $ScriptDir\build"

    # Print summary
    ""
    Write-Info "Build Summary:"
    Write-Info "  Library: build\$BuildType\container_system.lib"

    if ($BuildTests -eq "ON") {
        Write-Info "  Unit Tests: build\$BuildType\unit_tests.exe"
        Write-Info "  Integration Tests: build\$BuildType\test_messaging_integration.exe"
        Write-Info "  Performance Tests: build\$BuildType\performance_tests.exe"
        Write-Info "  Benchmark Tests: build\$BuildType\benchmark_tests.exe"
    }

    if ($BuildExamples -eq "ON") {
        Write-Info "  Basic Example: build\examples\$BuildType\basic_container_example.exe"
        Write-Info "  Advanced Example: build\examples\$BuildType\advanced_container_example.exe"
        Write-Info "  Real World Scenarios: build\examples\$BuildType\real_world_scenarios.exe"
        if ($MessagingFeatures -eq "ON") {
            Write-Info "  Messaging Example: build\examples\$BuildType\messaging_integration_example.exe"
        }
    }

    if ($BuildSamples -eq "ON") {
        Write-Info "  Samples: build\samples\$BuildType\"
    }

    if ($BuildDocs -eq "ON") {
        Write-Info "  Documentation: build\documents\html\index.html"
    }

    ""
    Write-Info "Quick Start:"
    Write-Info "  Run basic example: .\build\examples\$BuildType\basic_container_example.exe"

    if ($BuildExamples -eq "ON") {
        Write-Info "  Run advanced demo: .\build\examples\$BuildType\advanced_container_example.exe"
        Write-Info "  Run scenarios demo: .\build\examples\$BuildType\real_world_scenarios.exe"
    }

    if ($BuildTests -eq "ON") {
        Write-Info "  Run all tests: cd build; ctest --build-config $BuildType"
    }
}

# Main execution
function Main {
    if ($Help) {
        Show-Help
    }

    Write-Info "Container System Build Script for Windows (PowerShell)"
    ""

    if (Invoke-Build) {
        Show-BuildSummary
        ""
        Write-Info "Build completed successfully!"
    }
    else {
        Write-Error-Custom "Build failed!"
        exit 1
    }
}

# Run main function
Main