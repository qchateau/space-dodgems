#!/bin/bash

mkdir -p build && cd build
if [ ! -e CMakeCache.txt ]; then
    CXX=clang++-10 CC=clang-10 cmake .. -DCMAKE_BUILD_TYPE=Release
fi
make -j$(nproc)
