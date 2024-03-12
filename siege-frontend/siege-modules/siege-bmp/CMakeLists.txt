cmake_minimum_required(VERSION 3.20)
project(siege-bmp)


find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.
file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE 3space::3space)
target_link_libraries(${PROJECT_NAME} PUBLIC siege-win32)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)