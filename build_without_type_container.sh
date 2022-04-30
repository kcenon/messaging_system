if [ ! -d "./build/" ]
then
    mkdir build
fi
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -DUSE_PYTHON=ON
make -B
#make install