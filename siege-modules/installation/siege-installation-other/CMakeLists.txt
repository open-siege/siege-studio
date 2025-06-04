cmake_minimum_required(VERSION 3.30)
project(siege-installation-other)

add_library(siege-installation-fakk-2 SHARED src/fakk-2.cpp src/fakk-2.def)
add_library(siege-installation-gulf-war SHARED src/gulf-war.cpp src/other.def)

get_property(INSTALL_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

foreach(EXT_TARGET IN ITEMS ${INSTALL_TARGETS})
    set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
    target_link_libraries(${EXT_TARGET} PRIVATE siege-platform)
endforeach()