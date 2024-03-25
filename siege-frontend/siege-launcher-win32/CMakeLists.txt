cmake_minimum_required(VERSION 3.20)
project(siege-launcher-win32)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(siege-launcher-win32 WIN32 siege-launcher-win32.cpp siege-launcher-win32.rc)
set_property(TARGET siege-launcher-win32 PROPERTY CXX_STANDARD 23)
set_property(TARGET siege-launcher-win32 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_compile_definitions(siege-launcher-win32 PRIVATE UNICODE)
target_include_directories(siege-launcher-win32 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-launcher-win32 PRIVATE siege-input siege-win32 msxml6 oleacc)

install(TARGETS siege-launcher-win32 CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
