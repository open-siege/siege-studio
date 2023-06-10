cmake_minimum_required(VERSION 3.17)
project(siege-studio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(siege-studio src/new_main.cpp)
set_property(TARGET siege-studio PROPERTY CXX_STANDARD 17)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-studio PRIVATE siege-interface)

install(TARGETS siege-studio CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
