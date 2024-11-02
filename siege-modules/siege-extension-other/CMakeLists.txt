cmake_minimum_required(VERSION 3.20)
project(siege-extension-other)

if (WIN32)
    file(GLOB LIB_SRC_FILES src/*.cpp)
    
    foreach(CPP_SRC IN ITEMS ${LIB_SRC_FILES})
       get_filename_component(MODULE_NAME ${CPP_SRC} NAME_WLE)
       add_library(siege-extension-${MODULE_NAME} SHARED ${CPP_SRC} src/other.def)
    endforeach()
    
    get_property(OTHER_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${OTHER_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension)
    endforeach()
endif()
