cmake_minimum_required(VERSION 3.28)
project(siege-extension-real-space)

if (WIN32)
    add_library(siege-extension-wing-commander-armada SHARED src/wing-commander-armada.cpp src/real-space.def)
    add_library(siege-extension-wing-commander-3 SHARED src/wing-commander-3.cpp src/real-space.def)
    add_library(siege-extension-wing-commander-4 SHARED src/wing-commander-4.cpp src/real-space.def)
    add_library(siege-extension-strike-commander SHARED src/strike-commander.cpp src/real-space.def)

    get_property(REAL_SPACE_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${REAL_SPACE_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
