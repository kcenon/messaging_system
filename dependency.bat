@ECHO OFF

cd ..
IF NOT EXIST "./vcpkg/" (
    git clone https://github.com/microsoft/vcpkg.git
)
cd vcpkg
git pull
call bootstrap-vcpkg.bat
vcpkg install lz4 fmt cpprestsdk cryptopp asio python3 crossguid libpqgtest --triplet x64-windows --recurse
vcpkg upgrade --no-dry-run
cd ..
cd messaging_system
