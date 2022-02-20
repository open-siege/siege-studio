project(darkstar-extender)
cmake_minimum_required(VERSION 3.17)

if(NOT CONAN_CMAKE_SYSTEM_PROCESSOR)
    set(CONAN_CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(MSVC AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR CONAN_CMAKE_SYSTEM_PROCESSOR STREQUAL "x86"))
    message(STATUS "Including Darkstar Extender targets")
    add_subdirectory(darkstar.proxy)
    add_library(darkstar-proxy::darkstar-proxy ALIAS darkstar-proxy)

    add_subdirectory(darkstar)
    add_subdirectory(mem)
    add_subdirectory(launcher)
    add_subdirectory(tester)
    add_subdirectory(tray-player)
endif()
