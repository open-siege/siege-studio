cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

add_subdirectory(siege-installer)
add_subdirectory(siege-input)
add_subdirectory(siege-interface)
add_subdirectory(siege-launcher)
add_subdirectory(siege-studio)

if (WIN32)
    add_subdirectory(siege-launcher-win32)
endif()