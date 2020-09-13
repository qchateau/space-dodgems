cmake_minimum_required(VERSION 3.10.2)
project(spaceinvader_server)

include(cmake/conan.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fcoroutines-ts -stdlib=libc++)
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static  -stdlib=libc++ -lc++abi")

# add this to have a working compile_commands.json ...
include_directories(SYSTEM /usr/lib/llvm-10/include/c++/v1/)

conan_cmake_run(
    BASIC_SETUP
    CMAKE_TARGETS
    REQUIRES
        fmt/7.0.1
        spdlog/1.8.0
        boost/1.74.0
        nlohmann_json/3.9.1
    INSTALL_ARGS
        --build missing
)

add_executable(
    server

    main.cpp
    listener.cpp
    session.cpp
    player.cpp
    world.cpp
)
target_link_libraries(
    server

    CONAN_PKG::fmt
    CONAN_PKG::spdlog
    CONAN_PKG::boost
    CONAN_PKG::nlohmann_json
)
target_link_options(
    server
    PUBLIC

    -static
    -stdlib=libc++
    -lc++abi
    -fuse-ld=lld
)
