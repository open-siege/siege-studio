cmake_minimum_required(VERSION 3.20)
project(siege-extension)

if (WIN32)
    add_library(detours STATIC IMPORTED)   
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/detours/detours.lib INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/detours)
    
    add_library(siege-extension-starsiege SHARED src/extension.cpp src/starsiege.cpp src/starsiege.def)
    set_target_properties(siege-extension-starsiege PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23)
    target_link_libraries(siege-extension-starsiege PRIVATE siege-win-desktop detours)

    add_library(siege-extension-tribes SHARED src/extension.cpp src/tribes.cpp src/tribes.def)
    set_target_properties(siege-extension-tribes PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23)
    target_link_libraries(siege-extension-tribes PRIVATE siege-win-desktop detours)

    add_library(siege-extension-sof SHARED src/extension.cpp src/sof.cpp src/sof.def)
    set_target_properties(siege-extension-sof PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-sof PRIVATE siege-win-desktop detours)
endif()
