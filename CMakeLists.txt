cmake_minimum_required(VERSION 3.16)
project(DarkstarDtsConverter)
include(conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

file(GLOB SRC_FILES src/*.cpp)
add_executable(convert-dts ${SRC_FILES})

include_directories(${CONAN_INCLUDE_DIRS})
include_directories(src)

conan_target_link_libraries(convert-dts)

target_compile_options(convert-dts PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Werror>
        )
