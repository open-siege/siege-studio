project(unvol)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(unvol src/unvol.cpp)
set_property(TARGET unvol PROPERTY CXX_STANDARD 23)
target_link_libraries(unvol siege-resource)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(unvol TBB::tbb)
endif()

install(TARGETS unvol
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
