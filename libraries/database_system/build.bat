@echo off
REM Database System Build Script for Windows
REM Advanced C++20 Database System with Multi-Backend Support

setlocal enabledelayedexpansion

REM Display banner
echo ============================================
echo       Database System Build Script
echo ============================================

REM Display help information
if "%1"=="--help" goto :show_help
if "%1"=="/?" goto :show_help

REM Default values
set "BUILD_TYPE=Release"
set "BUILD_TARGET=all"
set "CMAKE_ARGS="
set "CORES=%NUMBER_OF_PROCESSORS%"
set "VERBOSE=false"
set "CLEAN=false"
set "USE_VCPKG=true"
set "POSTGRESQL=true"
set "MYSQL=false"
set "SQLITE=false"
set "INSTALL=false"
set "PREFIX=C:\Program Files\DatabaseSystem"
set "GENERATOR=Visual Studio 17 2022"

REM Parse command line arguments
:parse_args
if "%1"=="" goto :end_parse
if "%1"=="--clean" (
    set "CLEAN=true"
    shift
    goto :parse_args
)
if "%1"=="--debug" (
    set "BUILD_TYPE=Debug"
    shift
    goto :parse_args
)
if "%1"=="--release" (
    set "BUILD_TYPE=Release"
    shift
    goto :parse_args
)
if "%1"=="--all" (
    set "BUILD_TARGET=all"
    shift
    goto :parse_args
)
if "%1"=="--lib-only" (
    set "BUILD_TARGET=database"
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_DATABASE_SAMPLES=OFF -DUSE_UNIT_TEST=OFF"
    shift
    goto :parse_args
)
if "%1"=="--samples" (
    set "BUILD_TARGET=samples"
    set "CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_DATABASE_SAMPLES=ON -DUSE_UNIT_TEST=OFF"
    shift
    goto :parse_args
)
if "%1"=="--tests" (
    set "BUILD_TARGET=tests"
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_UNIT_TEST=ON"
    shift
    goto :parse_args
)
if "%1"=="--with-postgresql" (
    set "POSTGRESQL=true"
    shift
    goto :parse_args
)
if "%1"=="--no-postgresql" (
    set "POSTGRESQL=false"
    shift
    goto :parse_args
)
if "%1"=="--with-mysql" (
    set "MYSQL=true"
    shift
    goto :parse_args
)
if "%1"=="--with-sqlite" (
    set "SQLITE=true"
    shift
    goto :parse_args
)
if "%1"=="--no-vcpkg" (
    set "USE_VCPKG=false"
    shift
    goto :parse_args
)
if "%1"=="--use-system-deps" (
    set "USE_VCPKG=false"
    shift
    goto :parse_args
)
if "%1"=="--cores" (
    set "CORES=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--verbose" (
    set "VERBOSE=true"
    shift
    goto :parse_args
)
if "%1"=="--install" (
    set "INSTALL=true"
    shift
    goto :parse_args
)
if "%1"=="--prefix" (
    set "PREFIX=%2"
    shift
    shift
    goto :parse_args
)
if "%1"=="--vs2019" (
    set "GENERATOR=Visual Studio 16 2019"
    shift
    goto :parse_args
)
if "%1"=="--vs2022" (
    set "GENERATOR=Visual Studio 17 2022"
    shift
    goto :parse_args
)
echo Unknown option: %1
echo Use --help for usage information
exit /b 1

:end_parse

REM Validation
where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake is not installed or not in PATH
    exit /b 1
)

REM Setup build directory
set "BUILD_DIR=build"
if "%BUILD_TYPE%"=="Debug" set "BUILD_DIR=build_debug"

echo Build Configuration:
echo   Build Type: %BUILD_TYPE%
echo   Build Target: %BUILD_TARGET%
if "%POSTGRESQL%"=="true" (echo   PostgreSQL: ON) else (echo   PostgreSQL: OFF)
if "%MYSQL%"=="true" (echo   MySQL: ON) else (echo   MySQL: OFF)
if "%SQLITE%"=="true" (echo   SQLite: ON) else (echo   SQLite: OFF)
if "%USE_VCPKG%"=="true" (echo   Use vcpkg: YES) else (echo   Use vcpkg: NO)
echo   Cores: %CORES%
echo   Generator: %GENERATOR%
echo.

REM Clean build directory if requested
if "%CLEAN%"=="true" (
    echo Cleaning build directory...
    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM Configure CMake options
set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%"
if "%POSTGRESQL%"=="true" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_POSTGRESQL=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_POSTGRESQL=OFF"
)
if "%MYSQL%"=="true" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_MYSQL=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_MYSQL=OFF"
)
if "%SQLITE%"=="true" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_SQLITE=ON"
) else (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DUSE_SQLITE=OFF"
)
set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"

if "%INSTALL%"=="true" (
    set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_INSTALL_PREFIX=%PREFIX%"
)

if "%USE_VCPKG%"=="true" (
    REM Check for vcpkg
    set "VCPKG_ROOT="
    if exist "vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=%CD%\vcpkg"
    ) else if exist "..\vcpkg\vcpkg.exe" (
        set "VCPKG_ROOT=%CD%\..\vcpkg"
    ) else if defined VCPKG_ROOT (
        REM Use environment variable
    ) else (
        echo WARNING: vcpkg not found, will try to install...
        call dependency.bat
        if errorlevel 1 (
            echo ERROR: Failed to install vcpkg
            set "USE_VCPKG=false"
        ) else (
            set "VCPKG_ROOT=%CD%\vcpkg"
        )
    )

    if "%USE_VCPKG%"=="true" if not "!VCPKG_ROOT!"=="" (
        set "CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=!VCPKG_ROOT!\scripts\buildsystems\vcpkg.cmake"
        echo Using vcpkg from: !VCPKG_ROOT!
    )
)

set "CMAKE_ARGS=%CMAKE_ARGS% -G \"%GENERATOR%\" -A x64"

REM Configure
echo Configuring...
if "%VERBOSE%"=="true" (
    echo CMake command: cmake -B %BUILD_DIR% %CMAKE_ARGS%
)

cd "%BUILD_DIR%"
cmake .. %CMAKE_ARGS%
if errorlevel 1 (
    echo ERROR: Configuration failed
    cd ..
    exit /b 1
)
cd ..

REM Build
echo Building...
set "BUILD_ARGS=--config %BUILD_TYPE% --parallel %CORES%"

if "%VERBOSE%"=="true" (
    set "BUILD_ARGS=%BUILD_ARGS% --verbose"
)

if not "%BUILD_TARGET%"=="all" (
    set "BUILD_ARGS=%BUILD_ARGS% --target %BUILD_TARGET%"
)

cmake --build "%BUILD_DIR%" %BUILD_ARGS%
if errorlevel 1 (
    echo ERROR: Build failed
    exit /b 1
)

REM Run tests if requested
if "%BUILD_TARGET%"=="tests" goto :run_tests
if "%BUILD_TARGET%"=="all" goto :run_tests
goto :skip_tests

:run_tests
echo Running tests...
cd "%BUILD_DIR%"
ctest --output-on-failure -C "%BUILD_TYPE%"
if errorlevel 1 (
    echo ERROR: Tests failed
    cd ..
    exit /b 1
)
cd ..

:skip_tests

REM Install if requested
if "%INSTALL%"=="true" (
    echo Installing...
    cmake --install "%BUILD_DIR%" --config "%BUILD_TYPE%"
    if errorlevel 1 (
        echo ERROR: Installation failed
        exit /b 1
    )
)

echo.
echo Build completed successfully!
echo Build artifacts location: %CD%\%BUILD_DIR%

if "%BUILD_TARGET%"=="all" (
    echo Sample programs:
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*usage*" 2>nul
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*demo*" 2>nul
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*example*" 2>nul
) else if "%BUILD_TARGET%"=="samples" (
    echo Sample programs:
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*usage*" 2>nul
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*demo*" 2>nul
    dir /b "%BUILD_DIR%\bin\%BUILD_TYPE%\*example*" 2>nul
)

if "%INSTALL%"=="true" (
    echo Installed to: %PREFIX%
)

goto :eof

:show_help
echo Usage: %0 [options]
echo.
echo Build Options:
echo   --clean           Perform a clean rebuild by removing the build directory
echo   --debug           Build in debug mode (default is release)
echo   --release         Build in release mode (default)
echo.
echo Target Options:
echo   --all             Build all targets (default)
echo   --lib-only        Build only the database library
echo   --samples         Build only the sample applications
echo   --tests           Build and run the unit tests
echo.
echo Database Options:
echo   --with-postgresql Enable PostgreSQL support (default)
echo   --no-postgresql   Disable PostgreSQL support
echo   --with-mysql      Enable MySQL support (future)
echo   --with-sqlite     Enable SQLite support (future)
echo.
echo Feature Options:
echo   --no-vcpkg        Skip vcpkg and use system libraries only
echo   --use-system-deps Use system-installed dependencies
echo.
echo General Options:
echo   --cores N         Use N cores for compilation (default: auto-detect)
echo   --verbose         Show detailed build output
echo   --install         Install after building
echo   --prefix PATH     Installation prefix (default: C:\Program Files\DatabaseSystem)
echo   --help            Display this help and exit
echo.
echo Compiler Options:
echo   --vs2019          Use Visual Studio 2019 generator
echo   --vs2022          Use Visual Studio 2022 generator (default)
echo.
echo Examples:
echo   %0                                    # Default build (Release, with PostgreSQL)
echo   %0 --debug --tests                   # Debug build with tests
echo   %0 --no-postgresql --lib-only        # Library only without PostgreSQL
echo   %0 --clean --release --install       # Clean release build with install

goto :eof