cmake_minimum_required(VERSION 3.20)
project(siege-extension)

if (WIN32)
    add_library(siege-extension-starsiege SHARED src/starsiege.cpp src/starsiege.def)
    set_property(TARGET siege-extension-starsiege PROPERTY CXX_STANDARD 23)
    set_target_properties(siege-extension-starsiege PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
    target_link_libraries(siege-extension-starsiege PRIVATE siege-platform)
endif()
