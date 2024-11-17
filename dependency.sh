#!/bin/bash

pushd .

if [ ! -d "./thread_system" ]; then
    echo "thread_system directory not found. Initializing submodule..."
    git submodule add https://your-repository-url/thread_system.git
    git submodule update --init
elif [ -z "$(ls -A ./thread_system)" ]; then
    echo "thread_system directory is empty. Updating submodule..."
    git submodule update --init
fi

if [ ! -z "$1" ]; then
    if [ "$1" == "--submodule" ]; then
        echo "Updating submodules to latest remote version..."
        git submodule update --remote
    fi
fi

if [ -f "./thread_system/dependency.sh" ]; then
    /bin/bash ./thread_system/dependency.sh
else
    echo "Error: dependency.sh not found in thread_system"
    exit 1
fi

cd ..
if [ -d "./vcpkg" ]; then
    cd vcpkg
    ./vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpq gtest --recurse
else
    echo "Error: vcpkg directory not found"
    exit 1
fi

popd
