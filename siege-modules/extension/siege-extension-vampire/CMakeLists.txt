cmake_minimum_required(VERSION 3.30)
project(siege-extension-vampire)

if (WIN32)
    add_library(siege-extension-vampire-shared STATIC src/vampire-shared.cpp)
    target_link_libraries(siege-extension-vampire-shared PUBLIC siege-extension-id-tech-shared)

    add_library(siege-extension-take-no-prisoners SHARED src/take-no-prisoners.cpp src/vampire.def)
    add_library(siege-extension-mageslayer SHARED src/mageslayer.cpp src/vampire.def)

    set(VAMPIRE_TARGETS siege-extension-take-no-prisoners
                        siege-extension-mageslayer)

    foreach(EXT_TARGET IN ITEMS ${VAMPIRE_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23 CXX_SCAN_FOR_MODULES ON)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension-vampire-shared siege-extension siege-content-id-tech-resource siege-content-id-tech-scripting)
    endforeach()

    set_target_properties(siege-extension-vampire-shared PROPERTIES CXX_STANDARD 23 CXX_SCAN_FOR_MODULES ON)
endif()
