cmake_minimum_required(VERSION 3.16)
project(StarsiegeLauncher)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(cpr REQUIRED)
find_package(Catch2 REQUIRED)

add_executable(starsiege-installer src/installer.cpp)
set_property(TARGET starsiege-installer PROPERTY CXX_STANDARD 17)
target_link_libraries(starsiege-installer PRIVATE cpr::cpr)

## Testing
add_executable(tests src/tests_main.test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2)

include(CTest)
include(Catch)
catch_discover_tests(tests)

## Install/Packaging Section
install(TARGETS starsiege-installer
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

