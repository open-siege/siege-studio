project(json-to-dts)
cmake_minimum_required(VERSION 3.28)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(json-to-dts src/convert_json.cpp)
set_property(TARGET json-to-dts PROPERTY CXX_STANDARD 23)
target_link_libraries(json-to-dts siege-content)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(json-to-dts TBB::tbb)
endif()

install(TARGETS json-to-dts
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
