cmake_minimum_required(VERSION 3.16)
project(launcher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(detours REQUIRED)

file(GLOB SRC_FILES src/*.cpp src/**/*.cpp)

include_directories(src)
include_directories(${detours_INCLUDE_DIRS})

add_executable(Starsiege.ext ${SRC_FILES})

target_link_libraries(Starsiege.ext PRIVATE ${detours_LIBRARIES})
target_compile_options(Starsiege.ext PRIVATE $<$<CONFIG:RELEASE>:/O2>)

add_executable(es3.ext ${SRC_FILES})

target_link_libraries(es3.ext PRIVATE ${detours_LIBRARIES})
target_compile_options(es3.ext PRIVATE $<$<CONFIG:RELEASE>:/O2>)