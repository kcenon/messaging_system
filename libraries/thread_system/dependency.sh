#!/bin/bash

ORIGINAL_DIR=$(pwd)

if [ "$(uname)" == "Darwin" ]; then
    brew update
    brew upgrade
    brew install pkg-config cmake doxygen ninja
    brew install autoconf automake autoconf-archive python3
elif [ "$(uname)" == "Linux" ]; then
    apt update
    apt upgrade -y

    apt install cmake build-essential gdb doxygen -y

    apt-get install curl zip unzip tar -y
    apt-get install pkg-config ninja-build python3 -y
    apt-get install autoconf automake autoconf-archive -y

    if [ $(uname -m) == "aarch64" ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=arm
    fi
fi

doxygen

pushd ..

if [ ! -d "./vcpkg/" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi

pushd vcpkg

git pull
./bootstrap-vcpkg.sh

popd
popd

cd "$ORIGINAL_DIR"