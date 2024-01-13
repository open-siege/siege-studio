project(dts-to-obj)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(dts-to-obj src/convert_dts.cpp)
set_property(TARGET dts-to-obj PROPERTY CXX_STANDARD 23)
target_link_libraries(dts-to-obj 3space::3space)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(dts-to-obj TBB::tbb)
endif()

install(TARGETS dts-to-obj
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
