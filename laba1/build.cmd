@echo off

echo Updating repository...
git pull

if not exist build (
    mkdir build
)

cd build

cmake -G "MinGW Makefiles" ..
cmake --build .

pause
