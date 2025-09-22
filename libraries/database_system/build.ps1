# Database System Build Script for Windows PowerShell
# Advanced C++20 Database System with Multi-Backend Support

param(
    [switch]$Help,
    [switch]$Clean,
    [switch]$Debug,
    [switch]$Release,
    [switch]$All,
    [switch]$LibOnly,
    [switch]$Samples,
    [switch]$Tests,
    [switch]$WithPostgreSQL,
    [switch]$NoPostgreSQL,
    [switch]$WithMySQL,
    [switch]$WithSQLite,
    [switch]$NoVcpkg,
    [switch]$UseSystemDeps,
    [int]$Cores = $env:NUMBER_OF_PROCESSORS,
    [switch]$Verbose,
    [switch]$Install,
    [string]$Prefix = "C:\Program Files\DatabaseSystem",
    [switch]$VS2019,
    [switch]$VS2022
)

# Display banner
function Show-Banner {
    Write-Host "============================================" -ForegroundColor Blue
    Write-Host "      Database System Build Script         " -ForegroundColor Blue
    Write-Host "============================================" -ForegroundColor Blue
}

# Display help information
function Show-Help {
    Write-Host "Usage: .\build.ps1 [options]" -ForegroundColor White
    Write-Host ""
    Write-Host "Build Options:" -ForegroundColor Yellow
    Write-Host "  -Clean            Perform a clean rebuild by removing the build directory"
    Write-Host "  -Debug            Build in debug mode (default is release)"
    Write-Host "  -Release          Build in release mode (default)"
    Write-Host ""
    Write-Host "Target Options:" -ForegroundColor Yellow
    Write-Host "  -All              Build all targets (default)"
    Write-Host "  -LibOnly          Build only the database library"
    Write-Host "  -Samples          Build only the sample applications"
    Write-Host "  -Tests            Build and run the unit tests"
    Write-Host ""
    Write-Host "Database Options:" -ForegroundColor Yellow
    Write-Host "  -WithPostgreSQL   Enable PostgreSQL support (default)"
    Write-Host "  -NoPostgreSQL     Disable PostgreSQL support"
    Write-Host "  -WithMySQL        Enable MySQL support (future)"
    Write-Host "  -WithSQLite       Enable SQLite support (future)"
    Write-Host ""
    Write-Host "Feature Options:" -ForegroundColor Yellow
    Write-Host "  -NoVcpkg          Skip vcpkg and use system libraries only"
    Write-Host "  -UseSystemDeps    Use system-installed dependencies"
    Write-Host ""
    Write-Host "General Options:" -ForegroundColor Yellow
    Write-Host "  -Cores N          Use N cores for compilation (default: auto-detect)"
    Write-Host "  -Verbose          Show detailed build output"
    Write-Host "  -Install          Install after building"
    Write-Host "  -Prefix PATH      Installation prefix (default: C:\Program Files\DatabaseSystem)"
    Write-Host "  -Help             Display this help and exit"
    Write-Host ""
    Write-Host "Compiler Options:" -ForegroundColor Yellow
    Write-Host "  -VS2019           Use Visual Studio 2019 generator"
    Write-Host "  -VS2022           Use Visual Studio 2022 generator (default)"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1                                    # Default build (Release, with PostgreSQL)"
    Write-Host "  .\build.ps1 -Debug -Tests                     # Debug build with tests"
    Write-Host "  .\build.ps1 -NoPostgreSQL -LibOnly            # Library only without PostgreSQL"
    Write-Host "  .\build.ps1 -Clean -Release -Install          # Clean release build with install"
}

# Function to run command with error handling
function Invoke-BuildCommand {
    param(
        [string]$Command,
        [string]$Description,
        [string]$WorkingDirectory = $PWD
    )

    if ($Verbose) {
        Write-Host "Running: $Command" -ForegroundColor Blue
    }

    $originalLocation = Get-Location
    try {
        Set-Location $WorkingDirectory
        Invoke-Expression $Command
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code $LASTEXITCODE"
        }
        Write-Host "✅ $Description" -ForegroundColor Green
    }
    catch {
        Write-Host "❌ Failed: $Description" -ForegroundColor Red
        Write-Host "Error: $_" -ForegroundColor Red
        Set-Location $originalLocation
        exit 1
    }
    finally {
        Set-Location $originalLocation
    }
}

# Main script
Show-Banner

if ($Help) {
    Show-Help
    exit 0
}

# Set default values
$BuildType = if ($Debug) { "Debug" } else { "Release" }
$BuildTarget = "all"
$CMakeArgs = @()
$UseVcpkg = -not ($NoVcpkg -or $UseSystemDeps)
$PostgreSQL = -not $NoPostgreSQL
$MySQL = $WithMySQL
$SQLite = $WithSQLite
$Generator = if ($VS2019) { "Visual Studio 16 2019" } else { "Visual Studio 17 2022" }

# Handle target options
if ($LibOnly) {
    $BuildTarget = "database"
    $CMakeArgs += "-DBUILD_DATABASE_SAMPLES=OFF", "-DUSE_UNIT_TEST=OFF"
}
elseif ($Samples) {
    $BuildTarget = "samples"
    $CMakeArgs += "-DBUILD_DATABASE_SAMPLES=ON", "-DUSE_UNIT_TEST=OFF"
}
elseif ($Tests) {
    $BuildTarget = "tests"
    $CMakeArgs += "-DUSE_UNIT_TEST=ON"
}

# Validation
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "❌ CMake is not installed or not in PATH" -ForegroundColor Red
    exit 1
}

# Setup build directory
$BuildDir = if ($BuildType -eq "Debug") { "build_debug" } else { "build" }

Write-Host "📋 Build Configuration:" -ForegroundColor Cyan
Write-Host "  Build Type: $BuildType" -ForegroundColor White
Write-Host "  Build Target: $BuildTarget" -ForegroundColor White
Write-Host "  PostgreSQL: $(if ($PostgreSQL) { 'ON' } else { 'OFF' })" -ForegroundColor White
Write-Host "  MySQL: $(if ($MySQL) { 'ON' } else { 'OFF' })" -ForegroundColor White
Write-Host "  SQLite: $(if ($SQLite) { 'ON' } else { 'OFF' })" -ForegroundColor White
Write-Host "  Use vcpkg: $(if ($UseVcpkg) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  Cores: $Cores" -ForegroundColor White
Write-Host "  Generator: $Generator" -ForegroundColor White
Write-Host ""

# Clean build directory if requested
if ($Clean -and (Test-Path $BuildDir)) {
    Write-Host "🧹 Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item $BuildDir -Recurse -Force
}

# Create build directory
if (-not (Test-Path $BuildDir)) {
    New-Item -ItemType Directory -Path $BuildDir | Out-Null
}

# Configure CMake options
$CMakeArgs += "-DCMAKE_BUILD_TYPE=$BuildType"
$CMakeArgs += "-DUSE_POSTGRESQL=$(if ($PostgreSQL) { 'ON' } else { 'OFF' })"
$CMakeArgs += "-DUSE_MYSQL=$(if ($MySQL) { 'ON' } else { 'OFF' })"
$CMakeArgs += "-DUSE_SQLITE=$(if ($SQLite) { 'ON' } else { 'OFF' })"
$CMakeArgs += "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

if ($Install) {
    $CMakeArgs += "-DCMAKE_INSTALL_PREFIX=`"$Prefix`""
}

if ($UseVcpkg) {
    # Check for vcpkg
    $VcpkgRoot = $null
    if (Test-Path "vcpkg\vcpkg.exe") {
        $VcpkgRoot = "$PWD\vcpkg"
    }
    elseif (Test-Path "..\vcpkg\vcpkg.exe") {
        $VcpkgRoot = "$PWD\..\vcpkg"
    }
    elseif ($env:VCPKG_ROOT -and (Test-Path "$env:VCPKG_ROOT\vcpkg.exe")) {
        $VcpkgRoot = $env:VCPKG_ROOT
    }
    else {
        Write-Host "⚠️  vcpkg not found, will try to install..." -ForegroundColor Yellow
        try {
            & ".\dependency.ps1"
            if ($LASTEXITCODE -eq 0) {
                $VcpkgRoot = "$PWD\vcpkg"
            }
        }
        catch {
            Write-Host "❌ Failed to install vcpkg" -ForegroundColor Red
            $UseVcpkg = $false
        }
    }

    if ($UseVcpkg -and $VcpkgRoot) {
        $CMakeArgs += "-DCMAKE_TOOLCHAIN_FILE=`"$VcpkgRoot\scripts\buildsystems\vcpkg.cmake`""
        Write-Host "✅ Using vcpkg from: $VcpkgRoot" -ForegroundColor Green
    }
}

$CMakeArgs += "-G", "`"$Generator`"", "-A", "x64"

# Configure
Write-Host "⚙️  Configuring..." -ForegroundColor Blue
if ($Verbose) {
    Write-Host "CMake command: cmake -B $BuildDir $($CMakeArgs -join ' ')" -ForegroundColor Blue
}

$configureCommand = "cmake -B `"$BuildDir`" $($CMakeArgs -join ' ')"
Invoke-BuildCommand -Command $configureCommand -Description "CMake configuration"

# Build
Write-Host "🔨 Building..." -ForegroundColor Blue
$BuildArgs = @("--config", $BuildType, "--parallel", $Cores)

if ($Verbose) {
    $BuildArgs += "--verbose"
}

if ($BuildTarget -ne "all") {
    $BuildArgs += "--target", $BuildTarget
}

$buildCommand = "cmake --build `"$BuildDir`" $($BuildArgs -join ' ')"
Invoke-BuildCommand -Command $buildCommand -Description "Project build"

# Run tests if requested
if ($BuildTarget -eq "tests" -or $BuildTarget -eq "all") {
    Write-Host "🧪 Running tests..." -ForegroundColor Blue
    $testCommand = "ctest --output-on-failure -C `"$BuildType`""
    Invoke-BuildCommand -Command $testCommand -Description "Unit tests" -WorkingDirectory $BuildDir
}

# Install if requested
if ($Install) {
    Write-Host "📦 Installing..." -ForegroundColor Blue
    $installCommand = "cmake --install `"$BuildDir`" --config `"$BuildType`""
    Invoke-BuildCommand -Command $installCommand -Description "Installation"
}

Write-Host ""
Write-Host "✅ Build completed successfully!" -ForegroundColor Green
Write-Host "📁 Build artifacts location: $PWD\$BuildDir" -ForegroundColor Cyan

if ($BuildTarget -eq "all" -or $BuildTarget -eq "samples") {
    Write-Host "🎯 Sample programs:" -ForegroundColor Cyan
    $samplePath = "$BuildDir\bin\$BuildType"
    if (Test-Path $samplePath) {
        Get-ChildItem $samplePath -Filter "*usage*" -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $($_.Name)" }
        Get-ChildItem $samplePath -Filter "*demo*" -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $($_.Name)" }
        Get-ChildItem $samplePath -Filter "*example*" -ErrorAction SilentlyContinue | ForEach-Object { Write-Host "  $($_.Name)" }
    }
}

if ($Install) {
    Write-Host "📦 Installed to: $Prefix" -ForegroundColor Cyan
}