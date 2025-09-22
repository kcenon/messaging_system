# Network System Installation Script for Windows
# Requires PowerShell 5.0 or later

param(
    [string]$InstallPrefix = "C:\Program Files\NetworkSystem",
    [string]$BuildType = "Release",
    [switch]$WithTests,
    [switch]$WithSamples,
    [int]$Jobs = 0,
    [switch]$Clean,
    [switch]$Uninstall,
    [switch]$Help
)

# Set error action preference
$ErrorActionPreference = "Stop"

# Color functions
function Write-Success {
    Write-Host "[NetworkSystem] " -ForegroundColor Green -NoNewline
    Write-Host $args[0]
}

function Write-Error-Message {
    Write-Host "[ERROR] " -ForegroundColor Red -NoNewline
    Write-Host $args[0]
}

function Write-Warning-Message {
    Write-Host "[WARNING] " -ForegroundColor Yellow -NoNewline
    Write-Host $args[0]
}

# Show help
function Show-Help {
    Write-Host @"
Network System Installation Script for Windows

Usage: .\install.ps1 [OPTIONS]

Options:
    -InstallPrefix PATH    Installation directory (default: C:\Program Files\NetworkSystem)
    -BuildType TYPE        Build type: Debug, Release, RelWithDebInfo (default: Release)
    -WithTests             Build tests
    -WithSamples           Build sample programs
    -Jobs N                Number of parallel jobs (default: auto-detect)
    -Clean                 Clean build directory before building
    -Uninstall             Uninstall Network System
    -Help                  Show this help message

Examples:
    .\install.ps1                                     # Default installation
    .\install.ps1 -InstallPrefix C:\libs -WithTests   # Install to C:\libs with tests
    .\install.ps1 -BuildType Debug -WithSamples       # Debug build with samples
    .\install.ps1 -Uninstall                          # Remove installation

Requirements:
    - Visual Studio 2019 or later (or MinGW-w64)
    - CMake 3.16 or later
    - Git (for submodules)
"@
}

# Check if running as administrator
function Test-Administrator {
    $currentUser = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentUser)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Detect build environment
function Get-BuildEnvironment {
    # Check for Visual Studio
    $vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vsWhere) {
        $vsPath = & $vsWhere -latest -property installationPath
        if ($vsPath) {
            Write-Success "Found Visual Studio at: $vsPath"
            return "VisualStudio"
        }
    }

    # Check for MinGW
    if (Get-Command gcc -ErrorAction SilentlyContinue) {
        Write-Success "Found MinGW/GCC"
        return "MinGW"
    }

    # Check for MSYS2
    if (Test-Path "C:\msys64\mingw64\bin\gcc.exe") {
        Write-Success "Found MSYS2 MinGW-w64"
        $env:PATH = "C:\msys64\mingw64\bin;$env:PATH"
        return "MSYS2"
    }

    Write-Error-Message "No suitable C++ compiler found. Please install Visual Studio or MinGW-w64."
    exit 1
}

# Check dependencies
function Test-Dependencies {
    Write-Success "Checking dependencies..."

    # Check CMake
    if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
        Write-Error-Message "CMake not found. Please install CMake 3.16 or later."
        Write-Host "Download from: https://cmake.org/download/"
        exit 1
    }

    # Check Git
    if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
        Write-Warning-Message "Git not found. Submodules may not be available."
    }

    # Check Ninja (optional)
    if (Get-Command ninja -ErrorAction SilentlyContinue) {
        Write-Success "Found Ninja build system"
        return "Ninja"
    } else {
        Write-Warning-Message "Ninja not found. Using default generator (slower builds)."
        return $null
    }
}

# Install dependencies using vcpkg
function Install-Dependencies {
    Write-Success "Setting up dependencies..."

    # Check for vcpkg
    if ($env:VCPKG_ROOT) {
        $vcpkgExe = Join-Path $env:VCPKG_ROOT "vcpkg.exe"
    } else {
        $vcpkgExe = "C:\vcpkg\vcpkg.exe"
    }

    if (Test-Path $vcpkgExe) {
        Write-Success "Found vcpkg at: $vcpkgExe"

        Write-Success "Installing dependencies with vcpkg..."
        & $vcpkgExe install asio fmt --triplet x64-windows

        $script:VcpkgToolchain = Join-Path (Split-Path $vcpkgExe) "scripts\buildsystems\vcpkg.cmake"
    } else {
        Write-Warning-Message "vcpkg not found. Dependencies must be installed manually."
        Write-Host "Install vcpkg from: https://github.com/Microsoft/vcpkg"
    }
}

# Build Network System
function Build-NetworkSystem {
    Write-Success "Building Network System..."

    # Clean build directory if requested
    if ($Clean -and (Test-Path "build")) {
        Write-Success "Cleaning build directory..."
        Remove-Item -Path "build" -Recurse -Force
    }

    # Create build directory
    if (-not (Test-Path "build")) {
        New-Item -ItemType Directory -Path "build" | Out-Null
    }

    Push-Location "build"

    try {
        # Prepare CMake arguments
        $cmakeArgs = @(
            ".."
            "-DCMAKE_BUILD_TYPE=$BuildType"
            "-DCMAKE_INSTALL_PREFIX=`"$InstallPrefix`""
            "-DBUILD_TESTS=$($WithTests.IsPresent)"
            "-DBUILD_SAMPLES=$($WithSamples.IsPresent)"
            "-DBUILD_SHARED_LIBS=OFF"
        )

        # Add generator if available
        $generator = Test-Dependencies
        if ($generator -eq "Ninja") {
            $cmakeArgs += "-G", "Ninja"
        }

        # Add vcpkg toolchain if available
        if ($script:VcpkgToolchain) {
            $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=`"$script:VcpkgToolchain`""
        }

        # Configure
        Write-Success "Configuring with CMake..."
        & cmake $cmakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "CMake configuration failed"
        }

        # Determine number of parallel jobs
        if ($Jobs -eq 0) {
            $Jobs = (Get-CimInstance Win32_ComputerSystem).NumberOfLogicalProcessors
        }

        # Build
        Write-Success "Building with $Jobs parallel jobs..."
        & cmake --build . --config $BuildType --parallel $Jobs
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed"
        }

        # Run tests if enabled
        if ($WithTests) {
            Write-Success "Running tests..."
            & ctest -C $BuildType --output-on-failure
            if ($LASTEXITCODE -ne 0) {
                Write-Warning-Message "Some tests failed"
            }
        }
    }
    finally {
        Pop-Location
    }

    Write-Success "Build complete"
}

# Install Network System
function Install-NetworkSystem {
    Write-Success "Installing Network System to $InstallPrefix..."

    # Check if running as administrator for Program Files installation
    if ($InstallPrefix.StartsWith("C:\Program Files") -and -not (Test-Administrator)) {
        Write-Error-Message "Administrator privileges required to install to Program Files."
        Write-Host "Please run this script as Administrator or choose a different install location."
        exit 1
    }

    Push-Location "build"

    try {
        # Install
        & cmake --install . --config $BuildType
        if ($LASTEXITCODE -ne 0) {
            throw "Installation failed"
        }

        # Create NetworkSystem-config.cmake if not exists
        $configDir = Join-Path $InstallPrefix "lib\cmake\NetworkSystem"
        if (-not (Test-Path $configDir)) {
            New-Item -ItemType Directory -Path $configDir -Force | Out-Null
        }

        # Add to PATH (optional)
        $binDir = Join-Path $InstallPrefix "bin"
        if (Test-Path $binDir) {
            $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
            if ($currentPath -notlike "*$binDir*") {
                Write-Success "Adding $binDir to user PATH"
                [Environment]::SetEnvironmentVariable("Path", "$currentPath;$binDir", "User")
            }
        }

        # Create environment variable for CMake discovery
        [Environment]::SetEnvironmentVariable("NetworkSystem_DIR", $InstallPrefix, "User")
    }
    finally {
        Pop-Location
    }

    Write-Success "Installation complete!"
    Write-Host ""
    Write-Host "To use Network System in your CMake project:"
    Write-Host '    find_package(NetworkSystem REQUIRED)'
    Write-Host '    target_link_libraries(your_app NetworkSystem::NetworkSystem)'
    Write-Host ""
    Write-Host "You may need to restart your terminal for PATH changes to take effect."
}

# Uninstall Network System
function Uninstall-NetworkSystem {
    Write-Success "Uninstalling Network System from $InstallPrefix..."

    # Check if running as administrator for Program Files
    if ($InstallPrefix.StartsWith("C:\Program Files") -and -not (Test-Administrator)) {
        Write-Error-Message "Administrator privileges required to uninstall from Program Files."
        exit 1
    }

    # Remove installation directory
    if (Test-Path $InstallPrefix) {
        Write-Success "Removing $InstallPrefix"
        Remove-Item -Path $InstallPrefix -Recurse -Force
    }

    # Remove from PATH
    $binDir = Join-Path $InstallPrefix "bin"
    $currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
    if ($currentPath -like "*$binDir*") {
        Write-Success "Removing from PATH"
        $newPath = ($currentPath.Split(';') | Where-Object { $_ -ne $binDir }) -join ';'
        [Environment]::SetEnvironmentVariable("Path", $newPath, "User")
    }

    # Remove environment variable
    [Environment]::SetEnvironmentVariable("NetworkSystem_DIR", $null, "User")

    Write-Success "Uninstall complete"
}

# Main execution
function Main {
    Write-Host "========================================"
    Write-Host "   Network System Installation Script   "
    Write-Host "========================================"
    Write-Host ""

    if ($Help) {
        Show-Help
        return
    }

    if ($Uninstall) {
        Uninstall-NetworkSystem
        return
    }

    $buildEnv = Get-BuildEnvironment
    Test-Dependencies | Out-Null

    # Ask to install dependencies
    $install = Read-Host "Do you want to install dependencies with vcpkg? (y/N)"
    if ($install -eq 'y' -or $install -eq 'Y') {
        Install-Dependencies
    }

    Build-NetworkSystem
    Install-NetworkSystem

    Write-Host ""
    Write-Success "Network System has been successfully installed!"
    Write-Success "Installation directory: $InstallPrefix"
    Write-Success "Build type: $BuildType"
}

# Run main function
Main