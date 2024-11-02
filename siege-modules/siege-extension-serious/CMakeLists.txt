cmake_minimum_required(VERSION 3.20)
project(siege-extension-serious)

if (WIN32)
    add_library(siege-extension-serious-sam-first-encounter SHARED src/serious-sam-first-encounter.cpp src/serious.def)
    add_library(siege-extension-serious-sam-second-encounter SHARED src/serious-sam-second-encounter.cpp src/serious.def)
    
    get_property(SERIOUS_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${SERIOUS_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
