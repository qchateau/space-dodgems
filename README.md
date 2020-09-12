# experimental-ws-game

Trying out boost.beast with C++20 coroutines and websockets.

Maybe this will become a mini-game one day.

# How to run

```bash
# build the server
mkdir -p server/build
cd server/build
CXX=clang++-10 CC=clang-10 cmake .. -DCMAKE_BUILD_TYPE=Release
make -j
cd ../../

# serve the application
docker-compose up -d
```
