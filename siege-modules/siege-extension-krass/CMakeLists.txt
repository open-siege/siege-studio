cmake_minimum_required(VERSION 3.20)
project(siege-extension-krass)

if (WIN32)
    add_library(siege-extension-aquanox SHARED src/aquanox.cpp src/krass.def)
    add_library(siege-extension-aquanox-2 SHARED src/aquanox-2.cpp src/krass.def)
    
    get_property(UNREAL_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${UNREAL_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
