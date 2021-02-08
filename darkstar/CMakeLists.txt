cmake_minimum_required(VERSION 3.16)
project(darkstar)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(detours REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)

file(GLOB TEST_SRC_FILES src/*.test.cpp src/**/*.test.cpp src/**/**/*.test.cpp)
file(GLOB JSON_FILES src/config.json)
file(GLOB SRC_FILES src/*.cpp src/**/*.cpp)

list(REMOVE_ITEM SRC_FILES ${TEST_SRC_FILES})

add_library(darkstar SHARED ${SRC_FILES} src/darkstar.def)

add_custom_command(TARGET darkstar
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${JSON_FILES} ${CMAKE_BINARY_DIR}
        )

add_executable(tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
target_include_directories(tests PRIVATE ${Catch2_INCLUDES} ${GUI_INCLUDES})
target_link_libraries(tests PRIVATE Catch2::Catch2 ${GUI_LIBS})

include(CTest)
include(Catch)
catch_discover_tests(tests)

include_directories(../darkstar.proxy/src)
include_directories(src)
include_directories(${nlohmann_json_INCLUDE_DIRS})
include_directories(${detours_INCLUDE_DIRS})

target_link_libraries(darkstar PRIVATE ${detours_LIBRARIES} ${CMAKE_SOURCE_DIR}/libs/darkstar.proxy.lib)

target_compile_options(darkstar PRIVATE $<$<CONFIG:RELEASE>:/O2>)