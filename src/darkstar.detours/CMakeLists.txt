cmake_minimum_required(VERSION 3.16)
project(darkstar.detours)

find_package(detours REQUIRED)

add_library(darkstar.detours SHARED main.cpp darkstar.detours.def)

include_directories(${detours_INCLUDE_DIRS})

target_link_libraries(darkstar.detours PRIVATE ${detours_LIBRARIES})

target_compile_options(darkstar.detours PRIVATE $<$<CONFIG:RELEASE>:/O2>)