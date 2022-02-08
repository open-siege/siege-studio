project(json-to-dts)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(json-to-dts src/convert_json.cpp)
set_property(TARGET json-to-dts PROPERTY CXX_STANDARD 17)
target_link_libraries(json-to-dts 3space::3space)

install(TARGETS json-to-dts
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
