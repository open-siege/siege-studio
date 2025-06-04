cmake_minimum_required(VERSION 3.30)
project(siege-installation-dynamix)

add_library(siege-installation-starsiege SHARED src/starsiege.cpp src/dynamix.def)
add_library(siege-installation-earthsiege SHARED src/earthsiege.cpp src/dynamix.def)
add_library(siege-installation-earthsiege-2 SHARED src/earthsiege-2.cpp src/earthsiege-2.def)
add_library(siege-installation-tribes SHARED src/tribes.cpp src/dynamix.def)
add_library(siege-installation-tribes-2 SHARED src/tribes-2.cpp src/dynamix.def)

get_property(INSTALL_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

foreach(EXT_TARGET IN ITEMS ${INSTALL_TARGETS})
    set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
    target_link_libraries(${EXT_TARGET} PRIVATE siege-platform)
endforeach()