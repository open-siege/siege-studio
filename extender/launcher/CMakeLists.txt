cmake_minimum_required(VERSION 3.16)
project(launcher)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(detours REQUIRED)

add_executable(Starsiege.ext src/Main.cpp)
target_link_libraries(Starsiege.ext PRIVATE detours::detours)
set_property(TARGET Starsiege.ext PROPERTY CXX_STANDARD 17)

add_executable(es3.ext src/Main.cpp)
target_link_libraries(es3.ext PRIVATE detours::detours)
set_property(TARGET es3.ext PROPERTY CXX_STANDARD 17)

add_executable(extender-tester.ext src/Main.cpp)
target_link_libraries(extender-tester.ext PRIVATE detours::detours)
set_property(TARGET extender-tester.ext PROPERTY CXX_STANDARD 17)

install(TARGETS Starsiege.ext es3.ext extender-tester.ext
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
