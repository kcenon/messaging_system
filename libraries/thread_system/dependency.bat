@echo off

pushd .
cd ..

REM Check if the vcpkg directory exists
if not exist ".\vcpkg\" (
    git clone https://github.com/microsoft/vcpkg.git
)

cd vcpkg

REM Update the vcpkg repository
git pull

REM Bootstrap vcpkg (setup vcpkg)
call bootstrap-vcpkg.bat

popd