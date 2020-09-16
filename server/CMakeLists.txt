cmake_minimum_required(VERSION 3.10.2)
project(server)

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
   file(
        DOWNLOAD "https://github.com/conan-io/cmake-conan/raw/v0.15/conan.cmake"
        "${CMAKE_BINARY_DIR}/conan.cmake"
        TLS_VERIFY ON
    )
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fcoroutines-ts -stdlib=libc++)

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

if (NOT CONAN_ONLY)
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
endif () # NOT CONAN_ONLY
