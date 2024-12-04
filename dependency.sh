#!/bin/bash

pushd .

init_and_update_submodule() {
    if [ -d "./thread_system/.git" ]; then
        echo "Removing existing git repository..."
        rm -rf thread_system/.git
    fi

    if ! git submodule status ./thread_system &>/dev/null; then
        if ! git submodule add --force https://github.com/kcenon/thread_system.git; then
            echo "Failed to add thread_system submodule"
            return 1
        fi
    fi

    if ! git submodule update --init --recursive; then
        echo "Failed to initialize submodule"
        return 1
    fi

    return 0
}

if [ ! -d "./thread_system" ]; then
    echo "thread_system directory not found. Initializing submodule..."
    if ! init_and_update_submodule; then
        echo "Failed to initialize thread_system"
        exit 1
    fi
elif [ -z "$(ls -A ./thread_system)" ] || [ ! -d "./thread_system/.git" ]; then
    echo "thread_system directory is empty or not properly initialized. Updating submodule..."
    if ! init_and_update_submodule; then
        echo "Failed to update thread_system"
        exit 1
    fi
fi

if [ ! -z "$1" ] && [ "$1" == "--submodule" ]; then
    echo "Updating submodules to latest remote version..."
    cd thread_system
    CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
    cd ..
    
    if ! git submodule update --remote --recursive --merge thread_system; then
        echo "Failed to update submodules to latest version"
        exit 1
    fi
    echo "Successfully updated submodule to latest version"
fi

if [ -f "./thread_system/dependency.sh" ]; then
    if ! /bin/bash ./thread_system/dependency.sh; then
        echo "Error: dependency.sh execution failed"
        exit 1
    fi
else
    echo "Error: dependency.sh not found in thread_system"
    exit 1
fi

cd ..
if [ -d "./vcpkg" ]; then
    cd vcpkg
    if ! ./vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpq gtest --recurse; then
        echo "Error: vcpkg install failed"
        exit 1
    fi
else
    echo "Error: vcpkg directory not found"
    exit 1
fi

popd