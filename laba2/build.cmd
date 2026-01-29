@echo off
echo Building laba2 (Windows)...

if not exist build (
    mkdir build
)

cd build

cmake -G "MinGW Makefiles" ..
cmake --build .

pause
