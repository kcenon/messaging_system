@echo off
setlocal EnableDelayedExpansion

:: Messaging System Build Script for Windows
:: Based on Thread System build script with additional messaging-specific features

:: Display banner
echo.
echo ============================================
echo       Messaging System Build Script
echo ============================================
echo.

:: Process command line arguments
set CLEAN_BUILD=0
set BUILD_DOCS=0
set CLEAN_DOCS=0
set BUILD_TYPE=Release
set BUILD_BENCHMARKS=0
set TARGET=all
set DISABLE_STD_FORMAT=0
set DISABLE_STD_JTHREAD=0
set DISABLE_STD_SPAN=0
set USE_LOCKFREE=0
set BUILD_CORES=0
set VERBOSE=0
set BUILD_CONTAINER=1
set BUILD_DATABASE=1
set BUILD_NETWORK=1
set BUILD_PYTHON=1
set VCPKG_ROOT=%USERPROFILE%\vcpkg

:: Parse command line arguments
:parse_args
if "%~1"=="" goto :end_parse
if "%~1"=="--clean" (
    set CLEAN_BUILD=1
    shift
    goto :parse_args
)
if "%~1"=="--debug" (
    set BUILD_TYPE=Debug
    shift
    goto :parse_args
)
if "%~1"=="--benchmark" (
    set BUILD_BENCHMARKS=1
    shift
    goto :parse_args
)
if "%~1"=="--all" (
    set TARGET=all
    shift
    goto :parse_args
)
if "%~1"=="--lib-only" (
    set TARGET=lib-only
    shift
    goto :parse_args
)
if "%~1"=="--samples" (
    set TARGET=samples
    shift
    goto :parse_args
)
if "%~1"=="--tests" (
    set TARGET=tests
    shift
    goto :parse_args
)
if "%~1"=="--thread-system" (
    set TARGET=thread-system
    shift
    goto :parse_args
)
if "%~1"=="--no-container" (
    set BUILD_CONTAINER=0
    shift
    goto :parse_args
)
if "%~1"=="--no-database" (
    set BUILD_DATABASE=0
    shift
    goto :parse_args
)
if "%~1"=="--no-network" (
    set BUILD_NETWORK=0
    shift
    goto :parse_args
)
if "%~1"=="--no-python" (
    set BUILD_PYTHON=0
    shift
    goto :parse_args
)
if "%~1"=="--docs" (
    set BUILD_DOCS=1
    shift
    goto :parse_args
)
if "%~1"=="--clean-docs" (
    set BUILD_DOCS=1
    set CLEAN_DOCS=1
    shift
    goto :parse_args
)
if "%~1"=="--no-format" (
    set DISABLE_STD_FORMAT=1
    shift
    goto :parse_args
)
if "%~1"=="--no-jthread" (
    set DISABLE_STD_JTHREAD=1
    shift
    goto :parse_args
)
if "%~1"=="--no-span" (
    set DISABLE_STD_SPAN=1
    shift
    goto :parse_args
)
if "%~1"=="--lockfree" (
    set USE_LOCKFREE=1
    shift
    goto :parse_args
)
if "%~1"=="--cores" (
    set BUILD_CORES=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--vcpkg-root" (
    set VCPKG_ROOT=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--verbose" (
    set VERBOSE=1
    shift
    goto :parse_args
)
if "%~1"=="--help" (
    goto :show_help
)
if "%~1"=="/?" (
    goto :show_help
)
echo Unknown option: %~1
goto :show_help

:end_parse

:: Check for dependencies
echo [STATUS] Checking build dependencies...

:: Check for CMake
where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH
    echo [WARNING] Please install CMake and add it to your PATH
    echo [WARNING] You can use 'dependency.bat' to install required dependencies
    exit /b 1
)

:: Check for Git
where git >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Git not found in PATH
    echo [WARNING] Please install Git and add it to your PATH
    echo [WARNING] You can use 'dependency.bat' to install required dependencies
    exit /b 1
)

:: Check for vcpkg
if not exist "%VCPKG_ROOT%\" (
    echo [WARNING] vcpkg not found at %VCPKG_ROOT%
    echo [WARNING] Running dependency script to set up vcpkg...
    
    if exist "dependency.bat" (
        call dependency.bat
        if errorlevel 1 (
            echo [ERROR] Failed to run dependency.bat
            exit /b 1
        )
    ) else (
        echo [ERROR] dependency.bat script not found
        exit /b 1
    )
)

:: Check for Python if building Python bindings
if %BUILD_PYTHON%==1 (
    where python >nul 2>nul
    if errorlevel 1 (
        echo [WARNING] Python not found but Python bindings were requested
        echo [WARNING] Python bindings will not be built
        set BUILD_PYTHON=0
    )
)

:: Check for doxygen if building docs
if %BUILD_DOCS%==1 (
    where doxygen >nul 2>nul
    if errorlevel 1 (
        echo [WARNING] Doxygen not found but documentation was requested
        echo [WARNING] Documentation will not be generated
        set BUILD_DOCS=0
    )
)

echo [SUCCESS] All dependencies are satisfied

:: Determine the number of cores to use for building
if %BUILD_CORES%==0 (
    :: Get the number of logical processors
    for /f "tokens=2 delims==" %%i in ('wmic cpu get NumberOfLogicalProcessors /value ^| findstr NumberOfLogicalProcessors') do set BUILD_CORES=%%i
    :: If detection failed, use a safe default
    if %BUILD_CORES%==0 set BUILD_CORES=2
)

echo [STATUS] Using %BUILD_CORES% cores for compilation

:: Clean build if requested
if %CLEAN_BUILD%==1 (
    echo [STATUS] Performing clean build...
    if exist build rmdir /s /q build
    if exist build-debug rmdir /s /q build-debug
)

:: Determine build directory
set BUILD_DIR=build
if "%BUILD_TYPE%"=="Debug" (
    set BUILD_DIR=build-debug
)

:: Create build directory if it doesn't exist
if not exist %BUILD_DIR% (
    echo [STATUS] Creating build directory...
    mkdir %BUILD_DIR%
)

:: Prepare CMake arguments
set CMAKE_ARGS=-DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

:: Add module flags
if %BUILD_CONTAINER%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_CONTAINER=OFF
)

if %BUILD_DATABASE%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_DATABASE=OFF
)

if %BUILD_NETWORK%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_NETWORK=OFF
)

if %BUILD_PYTHON%==0 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_PYTHON_BINDINGS=OFF
)

:: Add feature flags based on options
if %DISABLE_STD_FORMAT%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DSET_STD_FORMAT=OFF -DFORCE_FMT_FORMAT=ON
)

if %DISABLE_STD_JTHREAD%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DSET_STD_JTHREAD=OFF
)

if %DISABLE_STD_SPAN%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DSET_STD_SPAN=OFF
)

if %BUILD_BENCHMARKS%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_BENCHMARKS=ON
)

if %USE_LOCKFREE%==1 (
    set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_LOCKFREE_BY_DEFAULT=ON
)

:: Set test option
if "%TARGET%"=="tests" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_UNIT_TEST=ON
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DUSE_UNIT_TEST=OFF
)

:: Set samples option
if "%TARGET%"=="samples" (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_MESSAGING_SAMPLES=ON
) else (
    set CMAKE_ARGS=%CMAKE_ARGS% -DBUILD_MESSAGING_SAMPLES=OFF
)

:: Enter build directory
pushd %BUILD_DIR%

:: Run CMake configuration
echo [STATUS] Configuring project with CMake...
echo [STATUS] CMake arguments: %CMAKE_ARGS%
cmake .. %CMAKE_ARGS%

:: Check if CMake configuration was successful
if errorlevel 1 (
    echo [ERROR] CMake configuration failed. See the output above for details.
    popd
    exit /b 1
)

:: Determine build target based on option
set "BUILD_TARGET="
if "%TARGET%"=="all" (
    set "BUILD_TARGET=ALL_BUILD"
) else if "%TARGET%"=="lib-only" (
    set "BUILD_TARGET=container;database;network"
) else if "%TARGET%"=="thread-system" (
    set "BUILD_TARGET=thread_base;thread_pool;typed_thread_pool;logger;utilities;monitoring"
) else if "%TARGET%"=="samples" (
    :: Get available sample targets from messaging system
    set "BUILD_TARGET=basic_demo"
) else if "%TARGET%"=="tests" (
    :: Tests are disabled on Windows by default, build samples instead
    echo [WARNING] Unit tests may be disabled on Windows platform
    echo [STATUS] Attempting to build available test targets...
    set "BUILD_TARGET=container_test;network_test;database_test"
)

:: Set build verbosity
if %VERBOSE%==1 (
    set "CMAKE_BUILD_ARGS=--verbose"
) else (
    set "CMAKE_BUILD_ARGS="
)

:: Build the project
echo [STATUS] Building project in %BUILD_TYPE% mode...

:: Use MSBuild for Windows
set BUILD_FAILED=0
if not "%BUILD_TARGET%"=="" (
    :: Parse multiple targets separated by semicolons for Windows
    for %%t in (%BUILD_TARGET:;= %) do (
        echo [STATUS] Building target: %%t
        cmake --build . --config %BUILD_TYPE% --target %%t -- /maxcpucount:%BUILD_CORES% %CMAKE_BUILD_ARGS%
        if errorlevel 1 (
            echo [WARNING] Target %%t failed to build or does not exist
            set BUILD_FAILED=1
        )
    )
) else (
    cmake --build . --config %BUILD_TYPE% -- /maxcpucount:%BUILD_CORES% %CMAKE_BUILD_ARGS%
    if errorlevel 1 set BUILD_FAILED=1
)

:: Check if any build failed
if %BUILD_FAILED%==1 (
    echo [WARNING] Some targets failed to build. Check the warnings above.
    :: Don't exit with error if some targets succeeded
)

:: Run tests if requested
if "%TARGET%"=="tests" (
    echo [STATUS] Running available tests...
    
    :: Check if any test executables were built
    if not exist "bin\*test*.exe" (
        echo [WARNING] No test executables found. Make sure tests were built correctly.
    )
    
    :: Run CTest with detailed output
    echo [STATUS] Executing CTest...
    ctest -C %BUILD_TYPE% --output-on-failure --verbose
    
    if errorlevel 1 (
        echo [ERROR] Some tests failed. See the output above for details.
        
        :: Show failed tests
        echo [STATUS] Re-running failed tests with maximum detail...
        ctest -C %BUILD_TYPE% --rerun-failed --output-on-failure --verbose
        
        popd
        exit /b 1
    ) else (
        echo [SUCCESS] All tests passed!
        
        :: Display test summary if available
        for /f %%i in ('ctest -C %BUILD_TYPE% -N 2^>nul ^| find /c "Test #"') do (
            echo [STATUS] Test Summary:
            echo   Total tests: %%i
        )
    )
)

:: Return to original directory
popd

:: Show completion message
if %BUILD_FAILED%==0 (
    echo [SUCCESS] Build completed successfully!
) else (
    echo [INFO] Build completed with some warnings. Check the output above.
)

:: Generate documentation if requested
if %BUILD_DOCS%==1 (
    echo [STATUS] Generating Doxygen documentation...
    
    :: Create docs directory if it doesn't exist
    if not exist docs mkdir docs
    
    :: Clean docs if requested
    if %CLEAN_DOCS%==1 (
        echo [STATUS] Cleaning documentation directory...
        del /q docs\* 2>nul
        for /d %%x in (docs\*) do rmdir /s /q "%%x"
    )
    
    :: Check if Doxyfile exists
    if not exist Doxyfile (
        echo [WARNING] Doxyfile not found. Creating default configuration...
        doxygen -g
    )
    
    :: Run doxygen
    doxygen Doxyfile
    
    :: Check if documentation generation was successful
    if errorlevel 1 (
        echo [ERROR] Documentation generation failed. See the output above for details.
    ) else (
        echo [SUCCESS] Documentation generated successfully in the docs directory!
    )
)

:: Final success message
echo.
echo ============================================
echo     Messaging System Build Complete
echo ============================================
echo.

if exist %BUILD_DIR%\bin (
    echo Available executables:
    dir /b %BUILD_DIR%\bin\
)

echo Build configuration:
echo   Build type: %BUILD_TYPE%
echo   Target: %TARGET%

:: Display enabled modules
echo   Modules: 
if %BUILD_CONTAINER%==1 echo     - Container
if %BUILD_DATABASE%==1 echo     - Database
if %BUILD_NETWORK%==1 echo     - Network
if %BUILD_PYTHON%==1 echo     - Python bindings

if %BUILD_BENCHMARKS%==1 (
    echo   Benchmarks: Enabled
)

if %USE_LOCKFREE%==1 (
    echo   Lock-free: Enabled by default
)

if %BUILD_DOCS%==1 (
    echo   Documentation: Generated
)

exit /b 0

:show_help
echo Usage: %~nx0 [options]
echo.
echo Build Options:
echo   --clean           Perform a clean rebuild by removing the build directory
echo   --debug           Build in debug mode (default is release)
echo   --benchmark       Build with benchmarks enabled
echo.
echo Target Options:
echo   --all             Build all targets (default)
echo   --lib-only        Build only the core libraries
echo   --samples         Build only the sample applications
echo   --tests           Build and run the unit tests with detailed output
echo   --thread-system   Build only the thread system components
echo.
echo Module Options:
echo   --no-container    Disable container module
echo   --no-database     Disable database module
echo   --no-network      Disable network module
echo   --no-python       Disable Python bindings
echo.
echo Documentation Options:
echo   --docs            Generate Doxygen documentation
echo   --clean-docs      Clean and regenerate Doxygen documentation
echo.
echo Feature Options:
echo   --no-format       Disable std::format even if supported
echo   --no-jthread      Disable std::jthread even if supported
echo   --no-span         Disable std::span even if supported
echo   --lockfree        Enable lock-free implementations by default
echo.
echo General Options:
echo   --cores N         Use N cores for compilation (default: auto-detect)
echo   --verbose         Show detailed build output
echo   --vcpkg-root PATH Set custom vcpkg root directory
echo   --help            Display this help and exit
echo.
exit /b 0