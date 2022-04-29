if [ ! -d "./build/" ]
then
    mkdir build
fi
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE="../../vcpkg/scripts/buildsystems/vcpkg.cmake" -D__USE_PYTHON__=1 -D__USE_TYPE_CONTAINER__=1
make -B
#make install