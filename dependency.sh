#!/bin/bash
if [ "$(uname)" == "Darwin" ]; then
    brew install pkg-config autoconf cmake
elif [ "$(uname)" == "Linux" ]; then
    apt update
    apt upgrade -y

    apt install cmake build-essential gdb -y

    apt-get update
    apt-get upgrade -y

    apt-get install curl zip unzip tar ninja-build -y
    apt-get install swig pkg-config autoconf -y

    if [ $(egrep "^(VERSION_ID)=" /etc/os-release) != "VERSION_ID=\"22.04\"" ]; then
        apt-get install python3-pip -y
        pip3 install cmake
    fi

    if [ $(uname -m) == "aarch64" ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=arm
    fi
fi

cd ..

if [ ! -d "./vcpkg/" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi

cd vcpkg

git pull
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg upgrade --no-dry-run
./vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpq gtest

cd ..
