cmake_minimum_required(VERSION 3.20)
project(siege-extension-isi-motor)

if (WIN32)
    add_library(siege-extension-gt-legends SHARED src/gt-legends.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-gtr-fia SHARED src/gtr-fia.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-gtr-fia-2 SHARED src/gtr-fia-2.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-bmw-m3 SHARED src/bmw-m3.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-volvo SHARED src/volvo.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-rfactor SHARED src/rfactor.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-lexus SHARED src/lexus.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-superleague SHARED src/superleague.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-race SHARED src/race.cpp src/dll-main.cpp src/isi.def)
    add_library(siege-extension-race-07 SHARED src/race-07.cpp src/dll-main.cpp src/isi.def)

    get_property(ISI_MOTOR_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${ISI_MOTOR_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
