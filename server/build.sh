#!/bin/bash

export CXX=clang++-10
export CC=clang-10

mkdir -p build && cd build
if [ ! -e CMakeCache.txt ]; then
    CXX=clang++-10 CC=clang-10 cmake .. -DCMAKE_BUILD_TYPE=Release
fi
make -j$(nproc)
