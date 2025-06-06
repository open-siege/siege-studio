cmake_minimum_required(VERSION 3.30)
project(siege-input-filtration)

if (WIN32)
    add_library(${PROJECT_NAME} SHARED src/${PROJECT_NAME}.cpp src/${PROJECT_NAME}.def)
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-extension)
endif()
