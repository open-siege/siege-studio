cmake_minimum_required(VERSION 3.20)
project(siege-studio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(siege-studio WIN32 "siege-studio-main.cpp")
set_property(TARGET siege-studio PROPERTY CXX_STANDARD 23)
set_property(TARGET siege-studio PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_compile_definitions(siege-studio PRIVATE UNICODE)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-studio PRIVATE siege-input siege-win32 msxml6 oleacc)

install(TARGETS siege-studio CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
