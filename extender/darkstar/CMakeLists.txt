cmake_minimum_required(VERSION 3.16)
project(darkstar)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)
find_package(detours REQUIRED)
find_package(siege-input REQUIRED)
find_package(darkstar-proxy QUIET)

add_library(darkstar SHARED src/main.cpp
        src/System.cpp
        src/Registry.cpp
        src/Input/winmm.cpp
        src/Hook/Game.cpp src/Hook/GameConsole.cpp
        src/darkstar.def)
add_custom_command(TARGET darkstar
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/config.json ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(darkstar PRIVATE detours::detours nlohmann_json::nlohmann_json darkstar-proxy::darkstar-proxy siege-input::siege-input winmm dinput8)
target_include_directories(darkstar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET darkstar PROPERTY CXX_STANDARD 17)

add_executable(darkstar-tests src/TestsMain.test.cpp src/Registry.test.cpp src/Registry.cpp)
target_link_libraries(darkstar-tests PRIVATE Catch2::Catch2)
set_property(TARGET darkstar-tests PROPERTY CXX_STANDARD 17)

include(CTest)
include(Catch)
catch_discover_tests(darkstar-tests)

install(TARGETS darkstar
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)

install(FILES src/config.json DESTINATION lib)
