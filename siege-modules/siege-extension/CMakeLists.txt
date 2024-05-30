cmake_minimum_required(VERSION 3.20)
project(siege-extension)

if (WIN32)
    add_library(detours STATIC IMPORTED)   
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/detours/detours.lib INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/detours)

    add_library(siege-extension SHARED src/extension.cpp src/extension.def)
    
    set_property(TARGET siege-extension PROPERTY CXX_STANDARD 23)
    target_link_libraries(siege-extension PRIVATE siege-win-desktop detours)

    add_library(siege-extension-starsiege SHARED src/starsiege.cpp src/starsiege.def)
    set_property(TARGET siege-extension-starsiege PROPERTY CXX_STANDARD 23)
    set_target_properties(siege-extension-starsiege PROPERTIES VS_PLATFORM_TOOLSET ClangCL)
    target_link_libraries(siege-extension-starsiege PRIVATE siege-win-desktop detours)
endif()
