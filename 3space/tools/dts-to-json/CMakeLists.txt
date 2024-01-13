project(dts-to-json)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(dts-to-json src/convert_dts.cpp)
set_property(TARGET dts-to-json PROPERTY CXX_STANDARD 23)
target_link_libraries(dts-to-json 3space::3space)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(dts-to-json TBB::tbb)
endif()

install(TARGETS dts-to-json
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
