cmake_minimum_required(VERSION 3.20)
project(siege-exports)

if (WIN32)
    add_library(siege-exports-starsiege SHARED src/starsiege.cpp src/module.def)
    set_property(TARGET siege-exports-starsiege PROPERTY CXX_STANDARD 23)
    target_link_libraries(siege-exports-starsiege PRIVATE siege-platform)
endif()
