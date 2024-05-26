cmake_minimum_required(VERSION 3.20)
project(siege-3d-content)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/module.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-content)
else()
    add_library(${PROJECT_NAME} SHARED src/views/dts_controller.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-content)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)