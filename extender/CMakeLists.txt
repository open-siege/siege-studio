project(darkstar-extender)
cmake_minimum_required(VERSION 3.17)

if(MSVC AND (${CMAKE_SYSTEM_PROCESSOR} MATCHES "^x86" OR ${CONAN_CMAKE_SYSTEM_PROCESSOR} MATCHES "^x86"))
    add_subdirectory(darkstar.proxy)
    add_library(darkstar-proxy::darkstar-proxy ALIAS darkstar-proxy)

    add_subdirectory(darkstar)
    add_subdirectory(mem)
    add_subdirectory(launcher)
endif()
