# This is not the default file used to compile the library.
# conan build . is what should be used.
# However, this is present for compatibility with CLion,
# And to get warnings/errors from Visual C++.
# However GCC supports regparm, which can achieve a similar results to Borland __fastcall.

cmake_minimum_required(VERSION 3.16)
project(darkstar-extender)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/darkstar)
include_directories(packages/include)

file(GLOB SRC_FILES src/darkstar/**/*.cpp)

add_library(darkstar SHARED ${SRC_FILES} src/darkstar/darkstar_vs.def)

target_link_libraries(darkstar)
