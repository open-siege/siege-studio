cmake_minimum_required(VERSION 3.20)
project(siege-extension)

if (WIN32)
    add_library(detours STATIC IMPORTED)   
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/detours/detours.lib INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/detours)
    
    add_library(siege-extension STATIC src/input-filter.cpp src/extension.cpp)
    set_target_properties(siege-extension PROPERTIES CXX_STANDARD 23)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(siege-extension PUBLIC siege-win-desktop detours)

    add_library(siege-extension-input-filter-low-level-hook SHARED src/input-filter.cpp src/input-filter-low-level-hook.cpp src/filter.def)
    set_target_properties(siege-extension-input-filter-low-level-hook PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-input-filter-low-level-hook PRIVATE siege-win-desktop detours)

    add_library(siege-extension-input-filter-raw-input SHARED src/input-filter.cpp src/input-filter-raw-input.cpp src/filter.def)
    set_target_properties(siege-extension-input-filter-raw-input PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-input-filter-raw-input PRIVATE siege-win-desktop detours)

    add_library(siege-extension-generic SHARED src/generic-extension.cpp src/generic-extension.def)
    set_target_properties(siege-extension-generic PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-generic PRIVATE siege-extension)
endif()
