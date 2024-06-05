cmake_minimum_required(VERSION 3.20)
project(siege-extension-darkstar)

if (WIN32)
    add_library(siege-extension-starsiege SHARED src/starsiege.cpp src/starsiege.def)
    set_target_properties(siege-extension-starsiege PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23)
    target_link_libraries(siege-extension-starsiege PRIVATE siege-extension)

    add_library(siege-extension-tribes SHARED src/tribes.cpp src/tribes.def)
    set_target_properties(siege-extension-tribes PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23)
    target_link_libraries(siege-extension-tribes PRIVATE siege-extension)
endif()
