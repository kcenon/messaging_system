#!/bin/bash
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake"
make -B
export LC_ALL=C
unset LANGUAGE
./unittest/unittest
