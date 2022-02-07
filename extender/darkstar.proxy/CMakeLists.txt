cmake_minimum_required(VERSION 3.16)
project(darkstar.proxy)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(packages/include)

file(GLOB SRC_FILES src/*.cpp)

add_library(darkstar.proxy SHARED ${SRC_FILES} src/darkstar.proxy.def)
