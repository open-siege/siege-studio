cmake_minimum_required(VERSION 3.16)
project(launcher)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(detours REQUIRED)

file(GLOB SRC_FILES src/*.cpp src/**/*.cpp)

add_executable(Starsiege.ext ${SRC_FILES})
target_link_libraries(Starsiege.ext PRIVATE detours::detours)
target_include_directories(Starsiege.ext PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET Starsiege.ext PROPERTY CXX_STANDARD 17)

add_executable(es3.ext ${SRC_FILES})
target_link_libraries(es3.ext PRIVATE detours::detours)
target_include_directories(Starsiege.ext PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET es3.ext PROPERTY CXX_STANDARD 17)

install(TARGETS Starsiege.ext es3.ext
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
