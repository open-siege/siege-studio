cmake_minimum_required(VERSION 3.20)
project(siege-extension-isi-motor)

if (WIN32)
    set(ISI_WITH_CMD_FILES src/cmd.cpp src/dll-main.cpp src/isi-with-cmd.def)
    set(ISI_WITHOUT_CMD_FILES src/cmd.cpp src/dll-main.cpp src/isi-without-cmd.def)

    add_library(siege-extension-gt-legends SHARED src/gt-legends.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-gtr-fia SHARED src/gtr-fia.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-gtr-fia-2 SHARED src/gtr-fia-2.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-bmw-m3 SHARED src/bmw-m3.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-rfactor SHARED src/rfactor.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-superleague SHARED src/superleague.cpp ${ISI_WITH_CMD_FILES})
    add_library(siege-extension-lexus SHARED src/lexus.cpp ${ISI_WITH_CMD_FILES})
    
    add_library(siege-extension-volvo SHARED src/volvo.cpp ${ISI_WITHOUT_CMD_FILES})
    add_library(siege-extension-race SHARED src/race.cpp ${ISI_WITHOUT_CMD_FILES})
    add_library(siege-extension-race-07 SHARED src/race-07.cpp ${ISI_WITHOUT_CMD_FILES})

    get_property(ISI_MOTOR_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${ISI_MOTOR_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
