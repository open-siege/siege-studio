cmake_minimum_required(VERSION 3.20)
project(siege-ui-2d-content)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/module.def)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-content)


install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)