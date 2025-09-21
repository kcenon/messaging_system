@echo off
setlocal enabledelayedexpansion

REM Monitoring System Build Script for Windows
REM Based on Thread System build script

REM Display banner
echo ============================================
echo     Monitoring System Build Script
echo ============================================

REM Initialize variables
set CLEAN_BUILD=0
set BUILD_TYPE=Release
set BUILD_BENCHMARKS=0
set TARGET=all
set BUILD_CORES=%NUMBER_OF_PROCESSORS%
set VERBOSE=0
set SHOW_HELP=0
set BUILD_TESTS=0
set BUILD_SAMPLES=1

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse_args
if /i "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto :parse_args
)
if /i "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if /i "%~1"=="--benchmark" (
    set BUILD_BENCHMARKS=1
    shift
    goto :parse_args
)
if /i "%~1"=="--lib-only" (
    set TARGET=lib-only
    set BUILD_SAMPLES=0
    set BUILD_TESTS=0
    shift
    goto :parse_args
)
if /i "%~1"=="--samples" (
    set TARGET=samples
    set BUILD_SAMPLES=1
    set BUILD_TESTS=0
    shift
    goto :parse_args
)
if /i "%~1"=="--tests" (
    set TARGET=tests
    set BUILD_SAMPLES=0
    set BUILD_TESTS=1
    shift
    goto :parse_args
)
if /i "%~1"=="--cores" (
    set BUILD_CORES=%~2
    shift
    shift
    goto :parse_args
)
if /i "%~1"=="--verbose" (
    set VERBOSE=1
    shift
    goto :parse_args
)
if /i "%~1"=="--help" (
    set SHOW_HELP=1
    shift
    goto :parse_args
)
echo Unknown option: %~1
set SHOW_HELP=1
:end_parse_args

REM Show help if requested
if %SHOW_HELP%==1 (
    echo.
    echo Usage: build.bat [options]
    echo.
    echo Build Options:
    echo   --clean           Perform a clean rebuild
    echo   --debug           Build in debug mode ^(default is release^)
    echo   --benchmark       Build with benchmarks enabled
    echo.
    echo Target Options:
    echo   --lib-only        Build only the core libraries
    echo   --samples         Build only the sample applications
    echo   --tests           Build and run the unit tests
    echo.
    echo General Options:
    echo   --cores N         Use N cores for compilation
    echo   --verbose         Show detailed build output
    echo   --help            Display this help and exit
    echo.
    exit /b 0
)

echo Build configuration:
echo   Build Type: %BUILD_TYPE%
echo   Target: %TARGET%
echo   Cores: %BUILD_CORES%
if %BUILD_BENCHMARKS%==1 echo   Benchmarks: Enabled

REM Check for required tools
echo.
echo Checking for required tools...

where cmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: CMake not found. Please install CMake and add it to PATH.
    exit /b 1
)

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git not found. Please install Git and add it to PATH.
    exit /b 1
)

REM Check for Visual Studio or Build Tools
set VS_FOUND=0
set CMAKE_GENERATOR=""

REM Check for VS 2022
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set CMAKE_GENERATOR="Visual Studio 17 2022"
    set VCVARS="%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set CMAKE_GENERATOR="Visual Studio 17 2022"
    set VCVARS="%ProgramFiles%\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars64.bat"
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" (
    set VS_FOUND=1
    set CMAKE_GENERATOR="Visual Studio 17 2022"
    set VCVARS="%ProgramFiles%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
)

REM Check for VS 2019
if %VS_FOUND%==0 (
    if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat" (
        set VS_FOUND=1
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set VCVARS="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat" (
        set VS_FOUND=1
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set VCVARS="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Professional\VC\Auxiliary\Build\vcvars64.bat"
    ) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" (
        set VS_FOUND=1
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set VCVARS="%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
    )
)

if %VS_FOUND%==0 (
    echo ERROR: Visual Studio not found. Please install Visual Studio 2019 or 2022.
    exit /b 1
)

echo Found Visual Studio: %CMAKE_GENERATOR%

REM Initialize Visual Studio environment
echo Initializing Visual Studio environment...
call %VCVARS% >nul

REM Check for vcpkg
if not exist "..\vcpkg" (
    echo ERROR: vcpkg not found in parent directory.
    echo Please run dependency.bat first to set up vcpkg.
    exit /b 1
)

REM Clean build if requested
if %CLEAN_BUILD%==1 (
    echo Performing clean build...
    if exist build rmdir /s /q build
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo.
echo Configuring project with CMake...
set CMAKE_ARGS=-G %CMAKE_GENERATOR% -A x64
set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=../../vcpkg/scripts/buildsystems/vcpkg.cmake

if %BUILD_BENCHMARKS%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_BENCHMARKS=ON
)

if %BUILD_SAMPLES%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_SAMPLES=OFF
)

if %BUILD_TESTS%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=OFF
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_TESTS=ON
)

cmake .. %CMAKE_ARGS%
if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    cd ..
    exit /b 1
)

REM Build the project
echo.
echo Building project in %BUILD_TYPE% mode...

if %VERBOSE%==1 (
    cmake --build . --config %BUILD_TYPE% --parallel %BUILD_CORES% --verbose
) else (
    cmake --build . --config %BUILD_TYPE% --parallel %BUILD_CORES%
)

if errorlevel 1 (
    echo ERROR: Build failed.
    cd ..
    exit /b 1
)

REM Run tests if requested
if "%TARGET%"=="tests" (
    echo.
    echo Running tests...
    ctest -C %BUILD_TYPE% --output-on-failure
    if errorlevel 1 (
        echo WARNING: Some tests failed.
    ) else (
        echo All tests passed!
    )
)

cd ..

REM Success message
echo.
echo ============================================
echo    Monitoring System Build Complete
echo ============================================
echo.
echo Build type: %BUILD_TYPE%
echo Target: %TARGET%

if exist build\bin\%BUILD_TYPE% (
    echo.
    echo Available executables:
    dir /b build\bin\%BUILD_TYPE%\*.exe 2>nul
)

exit /b 0