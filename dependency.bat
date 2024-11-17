@echo off
setlocal enabledelayedexpansion

pushd %cd%

if not exist "thread_system\" (
    echo thread_system directory not found. Initializing submodule...
    git submodule add https://your-repository-url/thread_system.git
    if errorlevel 1 (
        echo Error: Failed to add submodule
        exit /b 1
    )
    git submodule update --init
    if errorlevel 1 (
        echo Error: Failed to initialize submodule
        exit /b 1
    )
) else (
    dir /a /b "thread_system" | findstr "^" > nul
    if errorlevel 1 (
        echo thread_system directory is empty. Updating submodule...
        git submodule update --init
        if errorlevel 1 (
            echo Error: Failed to update submodule
            exit /b 1
        )
    )
)

if not "%1"=="" (
    if "%1"=="--submodule" (
        echo Updating submodules to latest remote version...
        git submodule update --remote
        if errorlevel 1 (
            echo Error: Failed to update submodule to remote version
            exit /b 1
        )
    )
)

if exist "thread_system\dependency.sh" (
    echo Running dependency script...
    "C:\Program Files\Git\bin\bash.exe" thread_system/dependency.sh
    if errorlevel 1 (
        echo Error: Failed to run dependency script
        exit /b 1
    )
) else (
    echo Error: dependency.sh not found in thread_system
    exit /b 1
)

cd ..
if exist "vcpkg\" (
    cd vcpkg
    vcpkg install lz4:x64-windows fmt:x64-windows cpprestsdk:x64-windows ^
                 cryptopp:x64-windows asio:x64-windows python3:x64-windows ^
                 crossguid:x64-windows libpq:x64-windows gtest:x64-windows --recurse
    if errorlevel 1 (
        echo Error: Failed to install vcpkg packages
        exit /b 1
    )
) else (
    echo Error: vcpkg directory not found
    exit /b 1
)

popd

endlocal