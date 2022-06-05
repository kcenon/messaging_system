if [ "$(uname)" == "Darwin" ]; then
    brew install pkg-config
    brew install autoconf
elif [ "$(uname)" == "Linux" ]; then
    apt update
    apt upgrade -y

    apt install cmake build-essential gdb -y

    apt-get update
    apt-get upgrade -y

    apt-get install curl zip unzip tar ninja-build -y
    apt-get install pkg-config autoconf -y

    if [ $(egrep "^(VERSION_ID)=" /etc/os-release) != "VERSION_ID=\"22.04\"" ]; then
        apt-get install python3-pip -y
        pip3 install cmake
    fi

    if [ $(uname -m) != "aarch64" ]; then
        export VCPKG_FORCE_SYSTEM_BINARIES=arm
    fi
fi

cd ..

if [ ! -d "./vcpkg/" ]; then
    git clone https://github.com/microsoft/vcpkg.git
fi

cd vcpkg

if git checkout master &&
    git fetch origin master &&
    [ $(git rev-list HEAD ^origin --count) != 0 ] &&
    git merge origin/master
then
    ./bootstrap-vcpkg.sh
    ./vcpkg upgrade --no-dry-run
else
    if [ ! -f "./vcpkg" ]; then
        ./bootstrap-vcpkg.sh
        ./vcpkg integrate install
        ./vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpq gtest
    fi
fi

cd ..