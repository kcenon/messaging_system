if [ ! -d "./build/" ]; then
    rm -rf build
fi
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake"
make -B
#make install