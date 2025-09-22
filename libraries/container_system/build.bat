@echo off
setlocal enabledelayedexpansion

REM Container System Build Script for Windows
REM This script builds the container system library with all features, tests, and examples

REM Script directory
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%"

REM Default values
set BUILD_TYPE=Release
set BUILD_TESTS=ON
set BUILD_EXAMPLES=ON
set BUILD_SAMPLES=ON
set BUILD_DOCS=OFF
set MESSAGING_FEATURES=ON
set PERFORMANCE_METRICS=ON
set EXTERNAL_INTEGRATION=ON
set CLEAN_BUILD=false
set VERBOSE=false
set JOBS=%NUMBER_OF_PROCESSORS%
set PLATFORM=x64
set GENERATOR=Visual Studio 17 2022

echo [INFO] Container System Build Script for Windows
echo.

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--help" goto :show_help
if /i "%~1"=="-h" goto :show_help
if /i "%~1"=="--debug" set BUILD_TYPE=Debug
if /i "%~1"=="-d" set BUILD_TYPE=Debug
if /i "%~1"=="--clean" set CLEAN_BUILD=true
if /i "%~1"=="-c" set CLEAN_BUILD=true
if /i "%~1"=="--no-tests" set BUILD_TESTS=OFF
if /i "%~1"=="-t" set BUILD_TESTS=OFF
if /i "%~1"=="--no-examples" set BUILD_EXAMPLES=OFF
if /i "%~1"=="-E" set BUILD_EXAMPLES=OFF
if /i "%~1"=="--no-samples" set BUILD_SAMPLES=OFF
if /i "%~1"=="-s" set BUILD_SAMPLES=OFF
if /i "%~1"=="--docs" set BUILD_DOCS=ON
if /i "%~1"=="-D" set BUILD_DOCS=ON
if /i "%~1"=="--no-messaging" set MESSAGING_FEATURES=OFF
if /i "%~1"=="--no-metrics" set PERFORMANCE_METRICS=OFF
if /i "%~1"=="--no-integration" set EXTERNAL_INTEGRATION=OFF
if /i "%~1"=="--verbose" set VERBOSE=true
if /i "%~1"=="-v" set VERBOSE=true
if /i "%~1"=="--x86" set PLATFORM=Win32
if /i "%~1"=="--vs2019" set GENERATOR=Visual Studio 16 2019
if /i "%~1"=="--vs2022" set GENERATOR=Visual Studio 17 2022
if /i "%~1"=="--ninja" set GENERATOR=Ninja
if /i "%~1"=="-j" (
    set JOBS=%~2
    shift
)
if /i "%~1"=="--jobs" (
    set JOBS=%~2
    shift
)
shift
goto :parse_args
:args_done

goto :main

:show_help
echo Container System Build Script for Windows
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -h, --help              Show this help message
echo   -d, --debug             Build in Debug mode (default: Release)
echo   -c, --clean             Clean build (remove build directory first)
echo   -t, --no-tests          Don't build tests
echo   -E, --no-examples       Don't build examples
echo   -s, --no-samples        Don't build samples
echo   -D, --docs              Build documentation
echo   --no-messaging          Disable messaging features
echo   --no-metrics            Disable performance metrics
echo   --no-integration        Disable external integration
echo   -v, --verbose           Verbose output
echo   -j, --jobs N            Number of parallel jobs (default: %NUMBER_OF_PROCESSORS%)
echo   --x86                   Build for x86 (default: x64)
echo   --vs2019                Use Visual Studio 2019
echo   --vs2022                Use Visual Studio 2022 (default)
echo   --ninja                 Use Ninja generator
echo.
echo Features (enabled by default):
echo   - Messaging features (messaging integration)
echo   - Performance metrics (monitoring and analytics)
echo   - External integration (callback system)
echo   - Unit tests and integration tests
echo   - Examples and samples
echo.
echo Requirements:
echo   - Visual Studio 2019 or later with C++ workload
echo   - CMake 3.16 or later
echo   - vcpkg (for dependencies)
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

:check_cmake
cmake --version >nul 2>&1
if %errorLevel% == 0 (
    exit /b 0
) else (
    call :print_error "CMake not found. Please install CMake or run dependency.bat"
    exit /b 1
)

:check_vcpkg
where vcpkg >nul 2>&1
if %errorLevel% == 0 (
    exit /b 0
)

REM Check common vcpkg locations
set VCPKG_PATHS=C:\vcpkg;C:\tools\vcpkg;C:\dev\vcpkg;%USERPROFILE%\vcpkg

for %%i in (%VCPKG_PATHS%) do (
    if exist "%%i\vcpkg.exe" (
        call :print_info "Found vcpkg at %%i"
        set VCPKG_ROOT=%%i
        set PATH=%%i;%PATH%
        exit /b 0
    )
)

call :print_error "vcpkg not found. Please install vcpkg or run dependency.bat"
exit /b 1

:check_visual_studio
REM Check if VS generator is available
cmake -G "%GENERATOR%" --help >nul 2>&1
if %errorLevel% == 0 (
    exit /b 0
) else (
    call :print_error "Visual Studio generator '%GENERATOR%' not available"
    call :print_info "Please install Visual Studio 2019 or later with C++ workload"
    exit /b 1
)

:main
REM Print build configuration
call :print_info "Container System Build Configuration:"
call :print_info "  Build Type: %BUILD_TYPE%"
call :print_info "  Platform: %PLATFORM%"
call :print_info "  Generator: %GENERATOR%"
call :print_info "  Build Tests: %BUILD_TESTS%"
call :print_info "  Build Examples: %BUILD_EXAMPLES%"
call :print_info "  Build Samples: %BUILD_SAMPLES%"
call :print_info "  Build Docs: %BUILD_DOCS%"
call :print_info "  Messaging Features: %MESSAGING_FEATURES%"
call :print_info "  Performance Metrics: %PERFORMANCE_METRICS%"
call :print_info "  External Integration: %EXTERNAL_INTEGRATION%"
call :print_info "  Parallel Jobs: %JOBS%"
call :print_info "  Clean Build: %CLEAN_BUILD%"
echo.

REM Check prerequisites
call :check_cmake
if %errorLevel% neq 0 exit /b 1

call :check_vcpkg
if %errorLevel% neq 0 exit /b 1

if not "%GENERATOR%"=="Ninja" (
    call :check_visual_studio
    if %errorLevel% neq 0 exit /b 1
)

REM Clean build directory if requested
if "%CLEAN_BUILD%"=="true" (
    call :print_info "Cleaning build directory..."
    if exist "build" rmdir /s /q "build"
)

REM Create build directory
call :print_info "Creating build directory..."
if not exist "build" mkdir "build"
cd /d "build"

REM Configure with CMake
call :print_step "Configuring with CMake..."

set CMAKE_ARGS=-DCMAKE_BUILD_TYPE=%BUILD_TYPE%
set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_UNIT_TEST=%BUILD_TESTS%
set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_CONTAINER_EXAMPLES=%BUILD_EXAMPLES%
set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_CONTAINER_SAMPLES=%BUILD_SAMPLES%
set CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_MESSAGING_FEATURES=%MESSAGING_FEATURES%
set CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_PERFORMANCE_METRICS=%PERFORMANCE_METRICS%
set CMAKE_ARGS=%CMAKE_ARGS% -DENABLE_EXTERNAL_INTEGRATION=%EXTERNAL_INTEGRATION%

REM Add vcpkg toolchain if available
if defined VCPKG_ROOT (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
)

if "%VERBOSE%"=="true" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DCMAKE_VERBOSE_MAKEFILE=ON
)

REM Set generator and platform
if "%GENERATOR%"=="Ninja" (
    cmake -G "Ninja" %CMAKE_ARGS% ..
) else (
    cmake -G "%GENERATOR%" -A %PLATFORM% %CMAKE_ARGS% ..
)

if %errorLevel% neq 0 (
    call :print_error "CMake configuration failed!"
    cd /d "%SCRIPT_DIR%"
    exit /b 1
)

REM Build
call :print_step "Building..."

if "%VERBOSE%"=="true" (
    cmake --build . --config %BUILD_TYPE% --parallel %JOBS% --verbose
) else (
    cmake --build . --config %BUILD_TYPE% --parallel %JOBS%
)

if %errorLevel% neq 0 (
    call :print_error "Build failed!"
    cd /d "%SCRIPT_DIR%"
    exit /b 1
)

REM Run tests if built
if "%BUILD_TESTS%"=="ON" (
    call :print_step "Running tests..."

    REM Run unit tests
    if exist "%BUILD_TYPE%\unit_tests.exe" (
        call :print_info "Running unit tests..."
        "%BUILD_TYPE%\unit_tests.exe"
        if !errorLevel! == 0 (
            call :print_info "Unit tests passed!"
        ) else (
            call :print_error "Unit tests failed!"
            cd /d "%SCRIPT_DIR%"
            exit /b 1
        )
    )

    REM Run integration tests
    if exist "%BUILD_TYPE%\test_messaging_integration.exe" (
        call :print_info "Running integration tests..."
        "%BUILD_TYPE%\test_messaging_integration.exe"
        if !errorLevel! == 0 (
            call :print_info "Integration tests passed!"
        ) else (
            call :print_error "Integration tests failed!"
            cd /d "%SCRIPT_DIR%"
            exit /b 1
        )
    )

    REM Run performance tests (but don't fail on performance issues)
    if exist "%BUILD_TYPE%\performance_tests.exe" (
        call :print_info "Running performance tests..."
        "%BUILD_TYPE%\performance_tests.exe"
        if !errorLevel! == 0 (
            call :print_info "Performance tests completed!"
        ) else (
            call :print_warning "Performance tests had issues (this is informational only)"
        )
    )

    REM Run all tests via CTest if available
    where ctest >nul 2>&1
    if !errorLevel! == 0 (
        call :print_info "Running all tests via CTest..."
        ctest --build-config %BUILD_TYPE% --output-on-failure
        if !errorLevel! == 0 (
            call :print_info "All CTest tests passed!"
        ) else (
            call :print_warning "Some CTest tests failed"
        )
    )
)

cd /d "%SCRIPT_DIR%"

call :print_info "Build completed successfully!"
call :print_info "Build artifacts are in: %SCRIPT_DIR%\build"

REM Print summary
echo.
call :print_info "Build Summary:"
call :print_info "  Library: build\%BUILD_TYPE%\container_system.lib"
if "%BUILD_TESTS%"=="ON" (
    call :print_info "  Unit Tests: build\%BUILD_TYPE%\unit_tests.exe"
    call :print_info "  Integration Tests: build\%BUILD_TYPE%\test_messaging_integration.exe"
    call :print_info "  Performance Tests: build\%BUILD_TYPE%\performance_tests.exe"
    call :print_info "  Benchmark Tests: build\%BUILD_TYPE%\benchmark_tests.exe"
)
if "%BUILD_EXAMPLES%"=="ON" (
    call :print_info "  Basic Example: build\examples\%BUILD_TYPE%\basic_container_example.exe"
    call :print_info "  Advanced Example: build\examples\%BUILD_TYPE%\advanced_container_example.exe"
    call :print_info "  Real World Scenarios: build\examples\%BUILD_TYPE%\real_world_scenarios.exe"
    if "%MESSAGING_FEATURES%"=="ON" (
        call :print_info "  Messaging Example: build\examples\%BUILD_TYPE%\messaging_integration_example.exe"
    )
)
if "%BUILD_SAMPLES%"=="ON" (
    call :print_info "  Samples: build\samples\%BUILD_TYPE%\"
)
if "%BUILD_DOCS%"=="ON" (
    call :print_info "  Documentation: build\documents\html\index.html"
)

echo.
call :print_info "Quick Start:"
call :print_info "  Run basic example: .\build\examples\%BUILD_TYPE%\basic_container_example.exe"
if "%BUILD_EXAMPLES%"=="ON" (
    call :print_info "  Run advanced demo: .\build\examples\%BUILD_TYPE%\advanced_container_example.exe"
    call :print_info "  Run scenarios demo: .\build\examples\%BUILD_TYPE%\real_world_scenarios.exe"
)
if "%BUILD_TESTS%"=="ON" (
    call :print_info "  Run all tests: cd build && ctest --build-config %BUILD_TYPE%"
)

echo.
call :print_info "Build completed successfully!"
pause
exit /b 0