cmake_minimum_required(VERSION 3.28)
project(siege-extension-cry-engine)

if (WIN32)
    add_library(siege-extension-far-cry SHARED src/far-cry.cpp src/cry.def)
    add_library(siege-extension-crysis SHARED src/crysis.cpp src/cry.def)
    add_library(siege-extension-crysis-warhead SHARED src/crysis-warhead.cpp src/cry.def)
    add_library(siege-extension-crysis-wars SHARED src/crysis-wars.cpp src/cry.def)

    get_property(CRY_ENGINE_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${CRY_ENGINE_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
