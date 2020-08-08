cmake_minimum_required(VERSION 3.16)
project(DarkstarDtsConverter)
include(conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

file(GLOB DTS_SRC_FILES src/dts-to-json/*.cpp)
file(GLOB JSON_SRC_FILES src/json-to-dts/*.cpp)
add_executable(dts-to-json ${DTS_SRC_FILES})
add_executable(json-to-dts ${JSON_SRC_FILES})

include_directories(${CONAN_INCLUDE_DIRS})
include_directories(src)

conan_target_link_libraries(dts-to-json)
conan_target_link_libraries(json-to-dts)

target_compile_options(dts-to-json PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
        )

target_compile_options(json-to-dts PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
        )
