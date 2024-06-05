cmake_minimum_required(VERSION 3.20)
project(siege-extension-id-tech)

if (WIN32)
    add_library(siege-extension-sof SHARED src/sof.cpp src/sof.def)
    set_target_properties(siege-extension-sof PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-sof PRIVATE siege-extension)
endif()
