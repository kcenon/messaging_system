#!/bin/bash

rm -rf lib
rm -rf bin
rm -rf build

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
make -B
export LC_ALL=C
unset LANGUAGE

cd ..

if [ -f "./bin/unittest" ]; then
    ./bin/unittest
fi

rm -rf build
