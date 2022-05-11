if [ ! "$(uname)" == "Darwin" ]; then
    apt update
    apt upgrade -y

    apt install cmake build-essential gdb -y

    apt-get update
    apt-get upgrade -y

    apt-get install curl zip unzip tar ninja-build -y
    apt-get install pkg-config autoconf -y
else
    brew install pkg-config
    brew install autoconf
fi
cd ..

if [ ! -d "./vcpkg/" ]
then
    git clone https://github.com/microsoft/vcpkg.git
fi
cd vcpkg
git pull
./bootstrap-vcpkg.sh
./vcpkg integrate install
./vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpq
./vcpkg upgrade --no-dry-run
cd ..
