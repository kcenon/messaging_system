@echo off
setlocal enabledelayedexpansion

pushd %cd%

:init_submodule
if exist "thread_system\.git" (
    echo Removing existing git repository...
    rmdir /s /q thread_system\.git
)
git submodule add --force https://github.com/kcenon/thread_system.git || (
    echo Failed to add submodule
    exit /b 1
)
git submodule update --init --recursive || exit /b 1
exit /b 0

:update_submodule
git submodule update --init --recursive || exit /b 1
exit /b 0

if not exist "thread_system\" (
    call :init_submodule || exit /b 1
) else (
    if not exist "thread_system\.git" (
        rmdir /s /q thread_system
        call :init_submodule || exit /b 1
    ) else (
        for /f "tokens=*" %%i in ('git -C thread_system remote get-url origin 2^>nul') do set "REMOTE_URL=%%i"
        if "!REMOTE_URL!"=="https://github.com/kcenon/thread_system.git" (
            git -C thread_system status >nul 2>&1 || (
                rmdir /s /q thread_system
                call :init_submodule || exit /b 1
            )
            call :update_submodule || exit /b 1
        ) else (
            echo Current remote: !REMOTE_URL!
            echo Expected: https://github.com/kcenon/thread_system.git
            choice /C YN /M "Remove and reinitialize?"
            if !ERRORLEVEL! equ 1 (
                rmdir /s /q thread_system
                call :init_submodule || exit /b 1
            ) else (
                exit /b 1
            )
        )
    )
)

if "%1"=="--submodule" (
    pushd thread_system
    for /f "tokens=*" %%i in ('git rev-parse --abbrev-ref HEAD') do set "CURRENT_BRANCH=%%i"
    popd
    git submodule update --remote --recursive --merge thread_system || exit /b 1
)

if exist "thread_system\dependency.bat" (
    call thread_system\dependency.bat || (
        echo Error: dependency.bat execution failed
        exit /b 1
    )
) else (
    echo Error: dependency.bat not found
    exit /b 1
)

cd ..
if exist "vcpkg\" (
    cd vcpkg
    vcpkg install lz4:x64-windows fmt:x64-windows cpprestsdk:x64-windows ^
                 cryptopp:x64-windows asio:x64-windows python3:x64-windows ^
                 crossguid:x64-windows libpq:x64-windows gtest:x64-windows --recurse || exit /b 1
) else (
    echo Error: vcpkg directory not found
    exit /b 1
)

popd
endlocal