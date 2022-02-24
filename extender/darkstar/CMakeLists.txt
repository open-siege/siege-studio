cmake_minimum_required(VERSION 3.16)
project(darkstar)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)
find_package(sfml REQUIRED)
find_package(detours REQUIRED)
find_package(darkstar-proxy QUIET)

add_subdirectory(music)
add_library(music-player::music-player ALIAS music-player)

file(GLOB TEST_SRC_FILES src/*.test.cpp src/**/*.test.cpp src/**/**/*.test.cpp)
file(GLOB SRC_FILES src/*.cpp src/**/*.cpp)

list(REMOVE_ITEM SRC_FILES ${TEST_SRC_FILES})

add_library(darkstar SHARED ${SRC_FILES} src/darkstar.def)
add_custom_command(TARGET darkstar
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/config.json ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(darkstar PRIVATE detours::detours nlohmann_json::nlohmann_json SFML::main SFML::system SFML::audio darkstar-proxy::darkstar-proxy music-player::music-player)
target_include_directories(darkstar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET darkstar PROPERTY CXX_STANDARD 17)

add_executable(darkstar-tests ${TEST_SRC_FILES})
target_link_libraries(darkstar-tests PRIVATE Catch2::Catch2)
set_property(TARGET darkstar-tests PROPERTY CXX_STANDARD 17)

include(CTest)
include(Catch)
catch_discover_tests(darkstar-tests)

install(TARGETS darkstar
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)

install(FILES src/config.json DESTINATION lib)
