# Database System Dependency Management Script for Windows PowerShell
# Advanced C++20 Database System with Multi-Backend Support

param(
    [switch]$Help,
    [switch]$SystemOnly,
    [switch]$VcpkgOnly,
    [switch]$All,
    [switch]$WithPostgreSQL,
    [switch]$WithMySQL,
    [switch]$WithSQLite,
    [switch]$NoDatabases,
    [switch]$WithDocs,
    [switch]$WithDebug,
    [switch]$WithProfiling,
    [switch]$Minimal,
    [switch]$Force,
    [switch]$DryRun,
    [switch]$Verbose
)

# Display banner
function Show-Banner {
    Write-Host "============================================" -ForegroundColor Blue
    Write-Host "    Database System Dependency Installer   " -ForegroundColor Blue
    Write-Host "============================================" -ForegroundColor Blue
}

# Display help information
function Show-Help {
    Write-Host "Usage: .\dependency.ps1 [options]" -ForegroundColor White
    Write-Host ""
    Write-Host "Installation Options:" -ForegroundColor Yellow
    Write-Host "  -SystemOnly       Install only system dependencies (no vcpkg)"
    Write-Host "  -VcpkgOnly        Install only vcpkg and its packages"
    Write-Host "  -All              Install all dependencies (default)"
    Write-Host ""
    Write-Host "Database Options:" -ForegroundColor Yellow
    Write-Host "  -WithPostgreSQL   Install PostgreSQL development libraries"
    Write-Host "  -WithMySQL        Install MySQL development libraries"
    Write-Host "  -WithSQLite       Install SQLite development libraries"
    Write-Host "  -NoDatabases      Skip database-specific dependencies"
    Write-Host ""
    Write-Host "Development Tools:" -ForegroundColor Yellow
    Write-Host "  -WithDocs         Install documentation tools (Doxygen, GraphViz)"
    Write-Host "  -WithDebug        Install debugging tools (Windows Debugging Tools)"
    Write-Host "  -WithProfiling    Install profiling tools"
    Write-Host "  -Minimal          Install only essential dependencies"
    Write-Host ""
    Write-Host "General Options:" -ForegroundColor Yellow
    Write-Host "  -Force            Force reinstallation of all packages"
    Write-Host "  -DryRun           Show what would be installed without installing"
    Write-Host "  -Verbose          Show detailed installation output"
    Write-Host "  -Help             Display this help and exit"
    Write-Host ""
    Write-Host "Prerequisites:" -ForegroundColor Yellow
    Write-Host "  - Windows 10/11 with PowerShell 5.1 or later"
    Write-Host "  - Visual Studio 2019 or later with C++ workload"
    Write-Host "  - Chocolatey package manager (recommended)"
    Write-Host ""
    Write-Host "Examples:" -ForegroundColor Yellow
    Write-Host "  .\dependency.ps1                                    # Install all dependencies"
    Write-Host "  .\dependency.ps1 -WithPostgreSQL -WithDocs         # Install with PostgreSQL and docs"
    Write-Host "  .\dependency.ps1 -Minimal -VcpkgOnly               # Minimal vcpkg installation"
    Write-Host "  .\dependency.ps1 -DryRun                           # Preview installation"
}

# Function to run command with dry-run support
function Invoke-DependencyCommand {
    param(
        [string]$Command,
        [string]$Description,
        [string]$WorkingDirectory = $PWD
    )

    if ($DryRun) {
        Write-Host "[DRY RUN] Would run: $Command" -ForegroundColor Cyan
        return $true
    }

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
        return $true
    }
    catch {
        Write-Host "❌ Failed: $Description" -ForegroundColor Red
        Write-Host "Error: $_" -ForegroundColor Red
        return $false
    }
    finally {
        Set-Location $originalLocation
    }
}

# Function to check if a tool is available
function Test-Tool {
    param([string]$ToolName)

    if (Get-Command $ToolName -ErrorAction SilentlyContinue) {
        Write-Host "✅ $ToolName is available" -ForegroundColor Green
        return $true
    } else {
        Write-Host "❌ $ToolName is not available" -ForegroundColor Red
        return $false
    }
}

# Function to install system dependencies
function Install-SystemDependencies {
    if (-not $InstallSystem) {
        return
    }

    Write-Host "📦 Installing system dependencies..." -ForegroundColor Blue

    # Check if chocolatey is installed
    if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
        Write-Host "⚠️  Chocolatey is not installed. Please install from https://chocolatey.org/" -ForegroundColor Yellow
        Write-Host "Or install dependencies manually:" -ForegroundColor Yellow
        Write-Host "  - Visual Studio 2019 or later with C++ workload"
        Write-Host "  - CMake >= 3.16"
        Write-Host "  - Git"
        return
    }

    # Essential build tools
    if (-not (Invoke-DependencyCommand -Command "choco install cmake git -y" -Description "Essential build tools")) {
        return
    }

    if (-not $Minimal) {
        Invoke-DependencyCommand -Command "choco install ninja -y" -Description "Ninja build system" | Out-Null
    }

    # Database libraries
    if ($InstallPostgreSQL) {
        Invoke-DependencyCommand -Command "choco install postgresql -y" -Description "PostgreSQL" | Out-Null
    }
    if ($InstallMySQL) {
        Invoke-DependencyCommand -Command "choco install mysql -y" -Description "MySQL" | Out-Null
    }

    # Development tools
    if ($InstallDebug) {
        Invoke-DependencyCommand -Command "choco install windbg -y" -Description "Windows Debugging Tools" | Out-Null
    }
    if ($InstallDocs) {
        Invoke-DependencyCommand -Command "choco install doxygen.install graphviz -y" -Description "Documentation tools" | Out-Null
    }
}

# Function to install vcpkg and packages
function Install-Vcpkg {
    if (-not $InstallVcpkg) {
        return
    }

    Write-Host "📦 Setting up vcpkg package manager..." -ForegroundColor Blue

    # Check if vcpkg already exists
    if (Test-Path "vcpkg") {
        if ($Force) {
            if (-not (Invoke-DependencyCommand -Command "Remove-Item vcpkg -Recurse -Force" -Description "Remove existing vcpkg")) {
                return
            }
        } else {
            Write-Host "⚠️  vcpkg directory already exists, updating..." -ForegroundColor Yellow
            if (-not (Invoke-DependencyCommand -Command "git pull" -Description "Update vcpkg" -WorkingDirectory "vcpkg")) {
                return
            }
        }
    }

    if (-not (Test-Path "vcpkg")) {
        if (-not (Invoke-DependencyCommand -Command "git clone https://github.com/microsoft/vcpkg.git" -Description "Clone vcpkg repository")) {
            return
        }
    }

    # Bootstrap vcpkg
    if (-not (Invoke-DependencyCommand -Command ".\bootstrap-vcpkg.bat" -Description "Bootstrap vcpkg" -WorkingDirectory "vcpkg")) {
        return
    }

    # Integrate vcpkg
    if (-not (Invoke-DependencyCommand -Command ".\vcpkg integrate install" -Description "Integrate vcpkg with build system" -WorkingDirectory "vcpkg")) {
        return
    }

    # Install C++ packages based on vcpkg.json if it exists
    if (Test-Path "vcpkg.json") {
        Write-Host "📋 Installing packages from vcpkg.json..." -ForegroundColor Cyan
        Invoke-DependencyCommand -Command ".\vcpkg install" -Description "Install packages from manifest" -WorkingDirectory "vcpkg" | Out-Null
    } else {
        Write-Host "📋 Installing essential C++ packages..." -ForegroundColor Cyan

        # Essential packages
        Invoke-DependencyCommand -Command ".\vcpkg install gtest" -Description "Google Test framework" -WorkingDirectory "vcpkg" | Out-Null
        Invoke-DependencyCommand -Command ".\vcpkg install fmt" -Description "fmt formatting library" -WorkingDirectory "vcpkg" | Out-Null
        Invoke-DependencyCommand -Command ".\vcpkg install spdlog" -Description "spdlog logging library" -WorkingDirectory "vcpkg" | Out-Null

        # Database packages
        if ($InstallPostgreSQL) {
            Invoke-DependencyCommand -Command ".\vcpkg install libpqxx" -Description "PostgreSQL C++ library" -WorkingDirectory "vcpkg" | Out-Null
        }
        if ($InstallMySQL) {
            Invoke-DependencyCommand -Command ".\vcpkg install libmysql" -Description "MySQL C++ library" -WorkingDirectory "vcpkg" | Out-Null
        }
        if ($InstallSQLite) {
            Invoke-DependencyCommand -Command ".\vcpkg install sqlite3" -Description "SQLite3 library" -WorkingDirectory "vcpkg" | Out-Null
        }

        # Additional useful packages
        if (-not $Minimal) {
            Invoke-DependencyCommand -Command ".\vcpkg install nlohmann-json" -Description "JSON library" -WorkingDirectory "vcpkg" | Out-Null
            Invoke-DependencyCommand -Command ".\vcpkg install catch2" -Description "Catch2 testing framework" -WorkingDirectory "vcpkg" | Out-Null
        }
    }
}

# Function to verify installation
function Test-Installation {
    Write-Host "🔍 Verifying installation..." -ForegroundColor Blue

    # Check essential tools
    $tools = @("cmake", "git")
    foreach ($tool in $tools) {
        Test-Tool -ToolName $tool | Out-Null
    }

    # Check vcpkg
    if (Test-Path "vcpkg") {
        Write-Host "✅ vcpkg is installed" -ForegroundColor Green
        if ($Verbose) {
            Write-Host "📋 Installed vcpkg packages:" -ForegroundColor Cyan
            $packages = & "vcpkg\vcpkg.exe" list 2>$null
            if ($packages) {
                $packages | ForEach-Object { Write-Host "  $_" }
            } else {
                Write-Host "  No packages installed yet"
            }
        }
    } else {
        Write-Host "⚠️  vcpkg is not installed" -ForegroundColor Yellow
    }

    # Check database libraries
    if ($InstallPostgreSQL) {
        # This is a simplified check - in reality, you'd want to check for development headers
        if (Get-Command pg_config -ErrorAction SilentlyContinue) {
            Write-Host "✅ PostgreSQL development libraries are available" -ForegroundColor Green
        } else {
            Write-Host "⚠️  PostgreSQL development libraries may not be available" -ForegroundColor Yellow
        }
    }
}

# Main script
Show-Banner

if ($Help) {
    Show-Help
    exit 0
}

# Set default values based on parameters
$InstallSystem = -not $VcpkgOnly
$InstallVcpkg = -not $SystemOnly
$InstallPostgreSQL = $WithPostgreSQL
$InstallMySQL = $WithMySQL
$InstallSQLite = $WithSQLite
$InstallDocs = $WithDocs
$InstallDebug = $WithDebug
$InstallProfiling = $WithProfiling

if ($NoDatabases) {
    $InstallPostgreSQL = $false
    $InstallMySQL = $false
    $InstallSQLite = $false
}

if ($All) {
    $InstallSystem = $true
    $InstallVcpkg = $true
}

Write-Host "📋 Dependency Installation Configuration:" -ForegroundColor Cyan
Write-Host "  OS: Windows" -ForegroundColor White
Write-Host "  Package Manager: chocolatey" -ForegroundColor White
Write-Host "  System Dependencies: $(if ($InstallSystem) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  vcpkg: $(if ($InstallVcpkg) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  PostgreSQL: $(if ($InstallPostgreSQL) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  MySQL: $(if ($InstallMySQL) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  SQLite: $(if ($InstallSQLite) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  Documentation: $(if ($InstallDocs) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  Debugging Tools: $(if ($InstallDebug) { 'YES' } else { 'NO' })" -ForegroundColor White
Write-Host "  Minimal Install: $(if ($Minimal) { 'YES' } else { 'NO' })" -ForegroundColor White
if ($DryRun) {
    Write-Host "  🔍 DRY RUN MODE - No actual installation" -ForegroundColor Yellow
}
Write-Host ""

# Install dependencies
Install-SystemDependencies
Install-Vcpkg

if (-not $DryRun) {
    Test-Installation

    Write-Host ""
    Write-Host "✅ Dependency installation completed!" -ForegroundColor Green
    Write-Host "📁 vcpkg location: $PWD\vcpkg" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "🎯 Next steps:" -ForegroundColor Cyan
    Write-Host "  1. Run .\build.ps1 to build the project"
    Write-Host "  2. Use CMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake for CMake"
    Write-Host "  3. Check .\build.ps1 -Help for build options"
} else {
    Write-Host ""
    Write-Host "🔍 Dry run completed. Use without -DryRun to install." -ForegroundColor Cyan
}