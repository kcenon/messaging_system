@echo off
REM Database System Dependency Management Script for Windows
REM Advanced C++20 Database System with Multi-Backend Support

setlocal enabledelayedexpansion

REM Display banner
echo ============================================
echo     Database System Dependency Installer
echo ============================================

REM Display help information
if "%1"=="--help" goto :show_help
if "%1"=="/?" goto :show_help

REM Default values
set "INSTALL_SYSTEM=true"
set "INSTALL_VCPKG=true"
set "INSTALL_POSTGRESQL=false"
set "INSTALL_MYSQL=false"
set "INSTALL_SQLITE=false"
set "INSTALL_DOCS=false"
set "INSTALL_DEBUG=false"
set "INSTALL_PROFILING=false"
set "MINIMAL=false"
set "FORCE=false"
set "DRY_RUN=false"
set "VERBOSE=false"

REM Parse command line arguments
:parse_args
if "%1"=="" goto :end_parse
if "%1"=="--system-only" (
    set "INSTALL_SYSTEM=true"
    set "INSTALL_VCPKG=false"
    shift
    goto :parse_args
)
if "%1"=="--vcpkg-only" (
    set "INSTALL_SYSTEM=false"
    set "INSTALL_VCPKG=true"
    shift
    goto :parse_args
)
if "%1"=="--all" (
    set "INSTALL_SYSTEM=true"
    set "INSTALL_VCPKG=true"
    shift
    goto :parse_args
)
if "%1"=="--with-postgresql" (
    set "INSTALL_POSTGRESQL=true"
    shift
    goto :parse_args
)
if "%1"=="--with-mysql" (
    set "INSTALL_MYSQL=true"
    shift
    goto :parse_args
)
if "%1"=="--with-sqlite" (
    set "INSTALL_SQLITE=true"
    shift
    goto :parse_args
)
if "%1"=="--no-databases" (
    set "INSTALL_POSTGRESQL=false"
    set "INSTALL_MYSQL=false"
    set "INSTALL_SQLITE=false"
    shift
    goto :parse_args
)
if "%1"=="--with-docs" (
    set "INSTALL_DOCS=true"
    shift
    goto :parse_args
)
if "%1"=="--with-debug" (
    set "INSTALL_DEBUG=true"
    shift
    goto :parse_args
)
if "%1"=="--with-profiling" (
    set "INSTALL_PROFILING=true"
    shift
    goto :parse_args
)
if "%1"=="--minimal" (
    set "MINIMAL=true"
    shift
    goto :parse_args
)
if "%1"=="--force" (
    set "FORCE=true"
    shift
    goto :parse_args
)
if "%1"=="--dry-run" (
    set "DRY_RUN=true"
    shift
    goto :parse_args
)
if "%1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
echo Unknown option: %1
echo Use --help for usage information
exit /b 1

:end_parse

echo Dependency Installation Configuration:
echo   OS: Windows
echo   Package Manager: chocolatey
if "%INSTALL_SYSTEM%"=="true" (echo   System Dependencies: YES) else (echo   System Dependencies: NO)
if "%INSTALL_VCPKG%"=="true" (echo   vcpkg: YES) else (echo   vcpkg: NO)
if "%INSTALL_POSTGRESQL%"=="true" (echo   PostgreSQL: YES) else (echo   PostgreSQL: NO)
if "%INSTALL_MYSQL%"=="true" (echo   MySQL: YES) else (echo   MySQL: NO)
if "%INSTALL_SQLITE%"=="true" (echo   SQLite: YES) else (echo   SQLite: NO)
if "%INSTALL_DOCS%"=="true" (echo   Documentation: YES) else (echo   Documentation: NO)
if "%INSTALL_DEBUG%"=="true" (echo   Debugging Tools: YES) else (echo   Debugging Tools: NO)
if "%MINIMAL%"=="true" (echo   Minimal Install: YES) else (echo   Minimal Install: NO)
if "%DRY_RUN%"=="true" echo   DRY RUN MODE - No actual installation
echo.

REM Install system dependencies
if "%INSTALL_SYSTEM%"=="true" goto :install_system
goto :skip_system

:install_system
echo Installing system dependencies...

REM Check if chocolatey is installed
where choco >nul 2>nul
if errorlevel 1 (
    echo WARNING: Chocolatey is not installed. Please install from https://chocolatey.org/
    echo Or install dependencies manually:
    echo   - Visual Studio 2019 or later with C++ workload
    echo   - CMake ^>= 3.16
    echo   - Git
    goto :skip_system
)

REM Essential build tools
call :run_command "choco install cmake git" "Essential build tools"
if not "%MINIMAL%"=="true" (
    call :run_command "choco install ninja" "Ninja build system"
)

REM Database libraries
if "%INSTALL_POSTGRESQL%"=="true" (
    call :run_command "choco install postgresql" "PostgreSQL"
)
if "%INSTALL_MYSQL%"=="true" (
    call :run_command "choco install mysql" "MySQL"
)

REM Development tools
if "%INSTALL_DEBUG%"=="true" (
    call :run_command "choco install windbg" "Windows Debugging Tools"
)
if "%INSTALL_DOCS%"=="true" (
    call :run_command "choco install doxygen.install graphviz" "Documentation tools"
)

:skip_system

REM Install vcpkg and packages
if "%INSTALL_VCPKG%"=="true" goto :install_vcpkg
goto :skip_vcpkg

:install_vcpkg
echo Setting up vcpkg package manager...

REM Check if vcpkg already exists
if exist "vcpkg" (
    if "%FORCE%"=="true" (
        call :run_command "rmdir /s /q vcpkg" "Remove existing vcpkg"
    ) else (
        echo WARNING: vcpkg directory already exists, updating...
        cd vcpkg
        call :run_command "git pull" "Update vcpkg"
        cd ..
    )
)

if not exist "vcpkg" (
    call :run_command "git clone https://github.com/microsoft/vcpkg.git" "Clone vcpkg repository"
)

REM Bootstrap vcpkg
cd vcpkg
call :run_command ".\bootstrap-vcpkg.bat" "Bootstrap vcpkg"

REM Integrate vcpkg
call :run_command ".\vcpkg integrate install" "Integrate vcpkg with build system"

REM Install C++ packages based on vcpkg.json if it exists
if exist "..\vcpkg.json" (
    echo Installing packages from vcpkg.json...
    call :run_command ".\vcpkg install" "Install packages from manifest"
) else (
    echo Installing essential C++ packages...

    REM Essential packages
    call :run_command ".\vcpkg install gtest" "Google Test framework"
    call :run_command ".\vcpkg install fmt" "fmt formatting library"
    call :run_command ".\vcpkg install spdlog" "spdlog logging library"

    REM Database packages
    if "%INSTALL_POSTGRESQL%"=="true" (
        call :run_command ".\vcpkg install libpqxx" "PostgreSQL C++ library"
    )
    if "%INSTALL_MYSQL%"=="true" (
        call :run_command ".\vcpkg install libmysql" "MySQL C++ library"
    )
    if "%INSTALL_SQLITE%"=="true" (
        call :run_command ".\vcpkg install sqlite3" "SQLite3 library"
    )

    REM Additional useful packages
    if not "%MINIMAL%"=="true" (
        call :run_command ".\vcpkg install nlohmann-json" "JSON library"
        call :run_command ".\vcpkg install catch2" "Catch2 testing framework"
    )
)

cd ..

:skip_vcpkg

REM Verify installation
if "%DRY_RUN%"=="true" goto :dry_run_complete

echo Verifying installation...

REM Check essential tools
call :check_tool cmake
call :check_tool git

REM Check vcpkg
if exist "vcpkg" (
    echo [92m✅ vcpkg is installed[0m
    if "%VERBOSE%"=="true" (
        cd vcpkg
        echo Installed vcpkg packages:
        .\vcpkg list 2>nul || echo No packages installed yet
        cd ..
    )
) else (
    echo [93m⚠️  vcpkg is not installed[0m
)

echo.
echo [92m✅ Dependency installation completed![0m
echo [96m📁 vcpkg location: %CD%\vcpkg[0m
echo.
echo [96m🎯 Next steps:[0m
echo   1. Run build.bat to build the project
echo   2. Use CMAKE_TOOLCHAIN_FILE=vcpkg\scripts\buildsystems\vcpkg.cmake for CMake
echo   3. Check build.bat --help for build options
goto :eof

:dry_run_complete
echo.
echo [96m🔍 Dry run completed. Use without --dry-run to install.[0m
goto :eof

REM Function to run command with dry-run support
:run_command
set "cmd=%~1"
set "description=%~2"

if "%DRY_RUN%"=="true" (
    echo [96m[DRY RUN][0m Would run: %cmd%
    goto :eof
)

if "%VERBOSE%"=="true" (
    echo [94mRunning:[0m %cmd%
)

%cmd%
if errorlevel 1 (
    echo [91m❌ Failed: %description%[0m
    exit /b 1
) else (
    echo [92m✅ %description%[0m
)
goto :eof

REM Function to check if a tool is available
:check_tool
where %1 >nul 2>nul
if errorlevel 1 (
    echo [91m❌ %1 is not available[0m
) else (
    echo [92m✅ %1 is available[0m
)
goto :eof

:show_help
echo Usage: %0 [options]
echo.
echo Installation Options:
echo   --system-only     Install only system dependencies (no vcpkg)
echo   --vcpkg-only      Install only vcpkg and its packages
echo   --all             Install all dependencies (default)
echo.
echo Database Options:
echo   --with-postgresql Install PostgreSQL development libraries
echo   --with-mysql      Install MySQL development libraries
echo   --with-sqlite     Install SQLite development libraries
echo   --no-databases    Skip database-specific dependencies
echo.
echo Development Tools:
echo   --with-docs       Install documentation tools (Doxygen, GraphViz)
echo   --with-debug      Install debugging tools (Windows Debugging Tools)
echo   --with-profiling  Install profiling tools
echo   --minimal         Install only essential dependencies
echo.
echo General Options:
echo   --force           Force reinstallation of all packages
echo   --dry-run         Show what would be installed without installing
echo   --verbose         Show detailed installation output
echo   --help            Display this help and exit
echo.
echo Prerequisites:
echo   - Windows 10/11 with PowerShell
echo   - Visual Studio 2019 or later with C++ workload
echo   - Chocolatey package manager (recommended)
echo.
echo Examples:
echo   %0                                    # Install all dependencies
echo   %0 --with-postgresql --with-docs     # Install with PostgreSQL and docs
echo   %0 --minimal --vcpkg-only            # Minimal vcpkg installation
echo   %0 --dry-run                         # Preview installation

goto :eof