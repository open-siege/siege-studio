project(open-siege)
cmake_minimum_required(VERSION 3.17)

if(NOT DEFINED CI)
    set(CI false)
endif()

if (CI MATCHES "^True" OR CI MATCHES "^true")
    message(STATUS "Finding package 3Space for CI build")
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
    find_package(3space REQUIRED)
endif()

if (NOT TARGET 3space::3space)
    message(STATUS "Including 3Space target")
    add_subdirectory(3space)
    add_library(3space::3space ALIAS 3space)
endif()

add_subdirectory(3space-studio)
add_subdirectory(tools)
add_subdirectory(siege-shell)
add_subdirectory(siege-input)
add_subdirectory(extender)
