cmake_minimum_required(VERSION 3.16)
project(darkstar-extender)

set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(packages/include)

file(GLOB SRC_FILES src/**/*.cpp)

file(GLOB JSON_FILES src/functions.json)

add_library(darkstar SHARED ${SRC_FILES} src/darkstar.def)

add_custom_command(TARGET darkstar
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${JSON_FILES} ${CMAKE_BINARY_DIR}
        )


target_link_libraries(darkstar)
