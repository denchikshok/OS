#!/bin/bash

echo "Building laba3 (Linux)"

mkdir -p build
cd build

cmake ..
cmake --build .
