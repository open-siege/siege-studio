cmake_minimum_required(VERSION 3.16)
project(siege)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)
find_package(detours REQUIRED)
find_package(siege-input REQUIRED)
find_package(siege-proxy QUIET)

add_library(siege SHARED src/main.cpp
        src/System.cpp
        src/Registry.cpp
        src/Input/winmm.cpp
        src/Hook/Game.cpp src/Hook/GameConsole.cpp
        src/siege.def)
add_custom_command(TARGET siege
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/config.json ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(siege PRIVATE detours::detours nlohmann_json::nlohmann_json siege-proxy::siege-proxy siege-input::siege-input winmm dinput8)
target_include_directories(siege PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
set_property(TARGET siege PROPERTY CXX_STANDARD 17)

add_executable(extender-tests src/TestsMain.test.cpp src/Registry.test.cpp src/Registry.cpp)
target_link_libraries(extender-tests PRIVATE Catch2::Catch2)
set_property(TARGET extender-tests PROPERTY CXX_STANDARD 17)

include(CTest)
include(Catch)
catch_discover_tests(extender-tests)

install(TARGETS siege
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)

install(FILES src/config.json DESTINATION lib)
