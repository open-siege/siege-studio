project(siege-extender)
cmake_minimum_required(VERSION 3.17)

if(NOT CONAN_CMAKE_SYSTEM_PROCESSOR)
    set(CONAN_CMAKE_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
endif()

if(MSVC AND (CMAKE_SYSTEM_PROCESSOR STREQUAL "x86" OR CONAN_CMAKE_SYSTEM_PROCESSOR STREQUAL "x86"))
    message(STATUS "Including Siege Extender targets")
    add_subdirectory(siege.proxy)
    add_library(siege-proxy::siege-proxy ALIAS siege-proxy)

    add_subdirectory(siege)
    add_subdirectory(mem)
    add_subdirectory(launcher)
    add_subdirectory(tester)
endif()
