cmake_minimum_required(VERSION 3.30)
project(siege-root)

set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/generators)
    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/build/generators)
endif()

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/libzt)
    set(BUILD_HOST_SELFTEST FALSE)
    # libzt is third-party and still declares VERSION 3.0; CMake 4.x requires >= 3.5
    set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    add_subdirectory(libzt)
endif()

add_subdirectory(siege-modules)
add_subdirectory(siege-tools)
add_subdirectory(siege-studio)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-launcher)
    add_subdirectory(siege-launcher)
endif()