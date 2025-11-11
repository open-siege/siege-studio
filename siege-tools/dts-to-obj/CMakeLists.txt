project(dts-to-obj)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(dts-to-obj src/convert_dts.cpp)
set_property(TARGET dts-to-obj PROPERTY CXX_STANDARD 23)
target_link_libraries(dts-to-obj siege-content-3space-3d)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(dts-to-obj TBB::tbb)
endif()


install(TARGETS dts-to-obj
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
