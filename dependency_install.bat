@ECHO OFF

cd ..
IF NOT EXIST "./vcpkg/" (
    git clone https://github.com/microsoft/vcpkg.git
)
cd vcpkg
git pull
call bootstrap-vcpkg.bat
vcpkg install lz4:x86-windows lz4:x64-windows
vcpkg install fmt:x86-windows fmt:x64-windows
vcpkg install cpprestsdk:x86-windows cpprestsdk:x64-windows
vcpkg install cryptopp:x86-windows cryptopp:x64-windows
vcpkg install asio:x86-windows asio:x64-windows
vcpkg install python3:x86-windows python3:x64-windows
vcpkg install crossguid:x86-windows crossguid:x64-windows
cd ..
cd messaging_system
