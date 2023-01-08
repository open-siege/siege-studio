cmake_minimum_required(VERSION 3.16)
project(siege-installer)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(cpr REQUIRED)
find_package(Catch2 REQUIRED)

add_executable(siege-installer src/installer.cpp src/games/games.cpp)
set_property(TARGET siege-installer PROPERTY CXX_STANDARD 17)
target_link_libraries(siege-installer PRIVATE cpr::cpr 3space::3space)
target_include_directories(siege-installer PRIVATE src)

## Testing
add_executable(tests src/tests_main.test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(tests)

## Install/Packaging Section
install(TARGETS siege-installer
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

