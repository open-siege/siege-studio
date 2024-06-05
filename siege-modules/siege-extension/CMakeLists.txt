cmake_minimum_required(VERSION 3.20)
project(siege-extension)

if (WIN32)
    add_library(detours STATIC IMPORTED)   
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/detours/detours.lib INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/detours)
    
    add_library(siege-extension STATIC src/extension.cpp)
    set_target_properties(siege-extension PROPERTIES CXX_STANDARD 23)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(siege-extension PUBLIC siege-win-desktop detours)
endif()
