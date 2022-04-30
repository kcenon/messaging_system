if [ ! -d "./build/" ]
then
    mkdir build
fi
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -DUSE_PYTHON=ON -DUSE_TYPE_CONTAINER=ON
make -B
#make install