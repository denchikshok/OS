#!/bin/bash

echo "Building laba2 (Linux)..."

mkdir -p build
cd build

cmake ..
cmake --build .
