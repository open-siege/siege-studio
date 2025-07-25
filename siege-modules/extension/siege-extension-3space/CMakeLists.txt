cmake_minimum_required(VERSION 3.30)
project(siege-extension-3space)

if (WIN32)
    add_library(siege-extension-starsiege SHARED src/starsiege.cpp src/starsiege.def)
    set_target_properties(siege-extension-starsiege PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23 CXX_SCAN_FOR_MODULES OFF)
    target_link_libraries(siege-extension-starsiege PRIVATE siege-extension)

    add_library(siege-extension-tribes SHARED src/tribes.cpp src/starsiege.def)
    set_target_properties(siege-extension-tribes PROPERTIES VS_PLATFORM_TOOLSET ClangCL CXX_STANDARD 23 CXX_SCAN_FOR_MODULES OFF)
    target_link_libraries(siege-extension-tribes PRIVATE siege-extension)

    add_library(siege-extension-mask-of-eternity SHARED src/moe.cpp src/darkstar.def)
    set_target_properties(siege-extension-mask-of-eternity PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-mask-of-eternity PRIVATE siege-extension)

    add_library(siege-extension-red-baron-3d-sp SHARED src/red-baron-3d-sp.cpp src/3space.def)
    set_target_properties(siege-extension-red-baron-3d-sp PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-red-baron-3d-sp PRIVATE siege-extension)

    add_library(siege-extension-red-baron-3d-mp SHARED src/red-baron-3d-mp.cpp src/3space.def)
    set_target_properties(siege-extension-red-baron-3d-mp PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-red-baron-3d-mp PRIVATE siege-extension)

endif()
