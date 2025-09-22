@echo off
setlocal enabledelayedexpansion

REM Container System Dependency Installation Script for Windows
REM This script installs all required dependencies for the Container System module

REM Script directory
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM Default values
set INSTALL_VCPKG=true
set INSTALL_CMAKE=true
set INSTALL_VISUAL_STUDIO=false
set FORCE_INSTALL=false

echo [INFO] Container System Dependency Installer for Windows
echo [INFO] OS: Windows
echo.

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--help" goto :show_help
if /i "%~1"=="-h" goto :show_help
if /i "%~1"=="--no-vcpkg" set INSTALL_VCPKG=false
if /i "%~1"=="--no-cmake" set INSTALL_CMAKE=false
if /i "%~1"=="--install-vs" set INSTALL_VISUAL_STUDIO=true
if /i "%~1"=="--force" set FORCE_INSTALL=true
shift
goto :parse_args
:args_done

goto :main

:show_help
echo Container System Dependency Installer for Windows
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -h, --help          Show this help message
echo   --no-vcpkg          Skip vcpkg installation
echo   --no-cmake          Skip CMake installation
echo   --install-vs        Install Visual Studio Build Tools
echo   --force             Force reinstallation of components
echo.
echo Prerequisites:
echo   - Windows 10/11 (x64)
echo   - Git for Windows
echo   - Visual Studio 2019 or later with C++ workload
echo.
exit /b 0

:print_info
echo [INFO] %~1
exit /b

:print_error
echo [ERROR] %~1
exit /b

:print_warning
echo [WARNING] %~1
exit /b

:print_step
echo [STEP] %~1
exit /b

:check_admin
net session >nul 2>&1
if %errorLevel% == 0 (
    exit /b 0
) else (
    exit /b 1
)

:check_git
git --version >nul 2>&1
if %errorLevel% == 0 (
    call :print_info "Git is available"
    exit /b 0
) else (
    call :print_error "Git is not installed or not in PATH"
    call :print_info "Please install Git for Windows from: https://git-scm.com/download/win"
    exit /b 1
)

:check_visual_studio
REM Check for Visual Studio or Build Tools
set VS_FOUND=false

REM Check for Visual Studio 2022
if exist "C:\Program Files\Microsoft Visual Studio\2022" set VS_FOUND=true
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022" set VS_FOUND=true

REM Check for Visual Studio 2019
if exist "C:\Program Files\Microsoft Visual Studio\2019" set VS_FOUND=true
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019" set VS_FOUND=true

REM Check for Build Tools
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools" set VS_FOUND=true
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools" set VS_FOUND=true

if "%VS_FOUND%"=="true" (
    call :print_info "Visual Studio or Build Tools found"
    exit /b 0
) else (
    call :print_warning "Visual Studio or Build Tools not found"
    exit /b 1
)

:install_visual_studio
call :print_step "Installing Visual Studio Build Tools..."
call :print_info "Downloading Visual Studio Build Tools installer..."

REM Download Visual Studio Build Tools installer
powershell -Command "Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vs_buildtools.exe' -OutFile 'vs_buildtools.exe'"

if not exist "vs_buildtools.exe" (
    call :print_error "Failed to download Visual Studio Build Tools installer"
    exit /b 1
)

call :print_info "Installing Visual Studio Build Tools with C++ workload..."
call :print_warning "This may take several minutes..."

REM Install with C++ workload
vs_buildtools.exe --quiet --wait --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.Windows10SDK.19041

if %errorLevel% neq 0 (
    call :print_error "Visual Studio Build Tools installation failed"
    exit /b 1
)

REM Cleanup
del vs_buildtools.exe

call :print_info "Visual Studio Build Tools installed successfully"
exit /b 0

:check_cmake
cmake --version >nul 2>&1
if %errorLevel% == 0 (
    call :print_info "CMake is available"
    exit /b 0
) else (
    exit /b 1
)

:install_cmake
call :print_step "Installing CMake..."

REM Check if we have admin rights for system-wide installation
call :check_admin
if %errorLevel% == 0 (
    set CMAKE_INSTALL_DIR=C:\Program Files\CMake
    call :print_info "Installing CMake system-wide..."
) else (
    set CMAKE_INSTALL_DIR=%USERPROFILE%\CMake
    call :print_info "Installing CMake for current user..."
)

REM Download CMake
call :print_info "Downloading CMake..."
set CMAKE_VERSION=3.28.1
set CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v%CMAKE_VERSION%/cmake-%CMAKE_VERSION%-windows-x86_64.msi

powershell -Command "Invoke-WebRequest -Uri '%CMAKE_URL%' -OutFile 'cmake-installer.msi'"

if not exist "cmake-installer.msi" (
    call :print_error "Failed to download CMake installer"
    exit /b 1
)

REM Install CMake
call :print_info "Installing CMake..."
msiexec /i cmake-installer.msi /quiet ADD_CMAKE_TO_PATH=System

if %errorLevel% neq 0 (
    call :print_error "CMake installation failed"
    exit /b 1
)

REM Cleanup
del cmake-installer.msi

call :print_info "CMake installed successfully"
exit /b 0

:check_vcpkg
where vcpkg >nul 2>&1
if %errorLevel% == 0 (
    call :print_info "vcpkg is available in PATH"
    exit /b 0
)

REM Check common vcpkg locations
set VCPKG_PATHS=C:\vcpkg;C:\tools\vcpkg;C:\dev\vcpkg;%USERPROFILE%\vcpkg

for %%i in (%VCPKG_PATHS%) do (
    if exist "%%i\vcpkg.exe" (
        call :print_info "Found vcpkg at %%i"
        set PATH=%%i;%PATH%
        exit /b 0
    )
)

exit /b 1

:install_vcpkg
call :print_step "Installing vcpkg..."

set VCPKG_ROOT=%USERPROFILE%\vcpkg

if exist "%VCPKG_ROOT%" (
    if "%FORCE_INSTALL%"=="true" (
        call :print_info "Removing existing vcpkg installation..."
        rmdir /s /q "%VCPKG_ROOT%"
    ) else (
        call :print_info "vcpkg directory already exists at %VCPKG_ROOT%"
        call :print_info "Use --force to reinstall"
        exit /b 1
    )
)

call :print_info "Cloning vcpkg to %VCPKG_ROOT%..."
git clone https://github.com/Microsoft/vcpkg.git "%VCPKG_ROOT%"

if %errorLevel% neq 0 (
    call :print_error "Failed to clone vcpkg repository"
    exit /b 1
)

cd /d "%VCPKG_ROOT%"

call :print_info "Bootstrapping vcpkg..."
call bootstrap-vcpkg.bat

if %errorLevel% neq 0 (
    call :print_error "Failed to bootstrap vcpkg"
    exit /b 1
)

REM Add to PATH for current session
set PATH=%VCPKG_ROOT%;%PATH%

call :print_info "vcpkg installed successfully at %VCPKG_ROOT%"
call :print_warning "Add %VCPKG_ROOT% to your system PATH for permanent access"

cd /d "%SCRIPT_DIR%"
exit /b 0

:install_cpp_dependencies
call :print_step "Installing C++ dependencies..."

REM Check if vcpkg.json exists
if not exist "vcpkg.json" (
    call :print_warning "vcpkg.json not found. Creating default configuration..."
    echo {> vcpkg.json
    echo     "name": "container-system",>> vcpkg.json
    echo     "description": "Advanced C++20 Container System with Thread-Safe Operations and Messaging Integration",>> vcpkg.json
    echo     "dependencies": [>> vcpkg.json
    echo         "fmt",>> vcpkg.json
    echo         "gtest",>> vcpkg.json
    echo         "benchmark">> vcpkg.json
    echo     ]>> vcpkg.json
    echo }>> vcpkg.json
)

REM Install dependencies
call :print_info "Installing vcpkg dependencies..."
vcpkg install --triplet x64-windows

if %errorLevel% neq 0 (
    call :print_error "Failed to install vcpkg dependencies"
    exit /b 1
)

call :print_info "C++ dependencies installed successfully"
exit /b 0

:install_test_framework
call :print_step "Checking test frameworks..."

REM Check if already installed via vcpkg
vcpkg list | findstr "gtest" >nul
if %errorLevel% == 0 (
    call :print_info "Google Test is already installed via vcpkg"
) else (
    call :print_warning "Installing Google Test..."
    vcpkg install gtest --triplet x64-windows
)

vcpkg list | findstr "benchmark" >nul
if %errorLevel% == 0 (
    call :print_info "Google Benchmark is already installed via vcpkg"
) else (
    call :print_warning "Installing Google Benchmark..."
    vcpkg install benchmark --triplet x64-windows
)

exit /b 0

:main
REM Check Git
call :check_git
if %errorLevel% neq 0 exit /b 1

REM Check Visual Studio
call :check_visual_studio
if %errorLevel% neq 0 (
    if "%INSTALL_VISUAL_STUDIO%"=="true" (
        call :install_visual_studio
        if %errorLevel% neq 0 exit /b 1
    ) else (
        call :print_error "Visual Studio or Build Tools required"
        call :print_info "Install Visual Studio 2019 or later with C++ workload"
        call :print_info "Or use --install-vs to install Build Tools automatically"
        exit /b 1
    )
)

REM Install CMake if needed
if "%INSTALL_CMAKE%"=="true" (
    call :check_cmake
    if %errorLevel% neq 0 (
        call :install_cmake
        if %errorLevel% neq 0 exit /b 1
    )
)

REM Install vcpkg if needed
if "%INSTALL_VCPKG%"=="true" (
    call :check_vcpkg
    if %errorLevel% neq 0 (
        call :install_vcpkg
        if %errorLevel% neq 0 exit /b 1
    )
)

REM Install C++ dependencies
call :install_cpp_dependencies
if %errorLevel% neq 0 exit /b 1

REM Install test frameworks
call :install_test_framework

echo.
call :print_info "All dependencies installed successfully!"
call :print_info "You can now build the project with build.bat"

echo.
call :print_step "For CMake integration, use:"
call :print_info "cmake -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake .."
call :print_info "Or set VCPKG_ROOT environment variable to: %VCPKG_ROOT%"

pause
exit /b 0