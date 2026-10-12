project(dts-convert)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(dts-convert src/convert_dts.cpp)
set_target_properties(dts-convert PROPERTIES CXX_STANDARD 23 CXX_SCAN_FOR_MODULES ON)
target_link_libraries(dts-convert siege-content-3space-3d)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(dts-convert TBB::tbb)
endif()


install(TARGETS dts-convert
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
