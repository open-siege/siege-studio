cmake_minimum_required(VERSION 3.30)
project(siege-extension)

if (WIN32)
    add_library(detours STATIC IMPORTED)   
    set_target_properties(detours PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/detours/detours.lib INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/detours)
    
    add_library(${PROJECT_NAME} STATIC src/${PROJECT_NAME}.cpp src/input_filter.cpp)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME} PUBLIC siege-win-desktop detours)
endif()
