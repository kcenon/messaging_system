@echo off
setlocal enabledelayedexpansion

REM Monitoring System Dependency Installation Script for Windows

REM Display banner
echo ============================================
echo   Monitoring System Dependency Installer
echo ============================================

REM Check if running as administrator
net session >nul 2>&1
if errorlevel 1 (
    echo WARNING: This script should be run as Administrator for best results.
    echo Some installations may fail without administrator privileges.
    echo.
    pause
)

REM Store original directory
set ORIGINAL_DIR=%CD%

REM Check for essential tools
echo Checking for required tools...

REM Check for Git
where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git is not installed or not in PATH.
    echo Please install Git for Windows from: https://git-scm.com/download/win
    exit /b 1
)
echo [OK] Git found

REM Check for CMake
where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH.
    echo Please install CMake from: https://cmake.org/download/
    exit /b 1
)
echo [OK] CMake found

REM Check for Visual Studio
set VS_FOUND=0
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1
if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" set VS_FOUND=1

if %VS_FOUND%==0 (
    echo ERROR: Visual Studio 2019 or 2022 is not installed.
    echo Please install Visual Studio with C++ development tools.
    echo Download from: https://visualstudio.microsoft.com/
    exit /b 1
)
echo [OK] Visual Studio found

REM Setup vcpkg
echo.
echo Setting up vcpkg...

REM Go to parent directory
cd ..

REM Check if vcpkg already exists
if not exist vcpkg (
    echo Cloning vcpkg repository...
    git clone https://github.com/microsoft/vcpkg.git
    if errorlevel 1 (
        echo ERROR: Failed to clone vcpkg repository
        cd %ORIGINAL_DIR%
        exit /b 1
    )
) else (
    echo vcpkg directory already exists, updating...
    cd vcpkg
    git pull
    cd ..
)

REM Bootstrap vcpkg
cd vcpkg
if not exist vcpkg.exe (
    echo Bootstrapping vcpkg...
    call bootstrap-vcpkg.bat
    if errorlevel 1 (
        echo ERROR: Failed to bootstrap vcpkg
        cd %ORIGINAL_DIR%
        exit /b 1
    )
) else (
    echo vcpkg.exe already exists, skipping bootstrap
)

echo [OK] vcpkg setup completed

REM Return to original directory
cd %ORIGINAL_DIR%

REM Create vcpkg.json if it doesn't exist
if not exist vcpkg.json (
    echo Creating vcpkg.json...
    (
        echo {
        echo   "name": "monitoring-system",
        echo   "version": "1.0.0",
        echo   "description": "Real-time performance monitoring system",
        echo   "dependencies": [
        echo     "gtest",
        echo     "benchmark"
        echo   ]
        echo }
    ) > vcpkg.json
)

REM Install vcpkg packages
echo.
echo Installing vcpkg dependencies...
..\vcpkg\vcpkg install --triplet x64-windows

if errorlevel 1 (
    echo ERROR: Failed to install some vcpkg dependencies
    exit /b 1
)

echo [OK] All vcpkg dependencies installed successfully

REM Check for optional tools
echo.
echo Checking for optional tools...

REM Check for Doxygen
where doxygen >nul 2>nul
if errorlevel 1 (
    echo [INFO] Doxygen not found. Documentation generation will not be available.
    echo        Download from: https://www.doxygen.nl/download.html
) else (
    echo [OK] Doxygen found
)

REM Check for Ninja
where ninja >nul 2>nul
if errorlevel 1 (
    echo [INFO] Ninja not found. Using MSBuild instead.
    echo        For faster builds, download from: https://ninja-build.org/
) else (
    echo [OK] Ninja found
)

REM Display summary
echo.
echo ============================================
echo      Dependency Installation Complete
echo ============================================
echo.
echo Summary of installed components:
echo   [Required]
echo   - Git: Found
echo   - CMake: Found
echo   - Visual Studio: Found
echo   - vcpkg: Installed
echo   - C++ Libraries: gtest, benchmark
echo.
echo   [Optional]
where doxygen >nul 2>nul
if errorlevel 1 (
    echo   - Doxygen: Not found
) else (
    echo   - Doxygen: Found
)
where ninja >nul 2>nul
if errorlevel 1 (
    echo   - Ninja: Not found
) else (
    echo   - Ninja: Found
)

echo.
echo Next steps:
echo   1. Run 'build.bat' to build the Monitoring System
echo   2. Run 'build.bat --help' to see all build options
echo   3. Run 'build.bat --tests' to build and run tests
echo.

pause
exit /b 0