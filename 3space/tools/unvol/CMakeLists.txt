project(unvol)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(unvol src/unvol.cpp)
set_property(TARGET unvol PROPERTY CXX_STANDARD 23)
target_link_libraries(unvol 3space::3space)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(unvol TBB::tbb)
endif()

install(TARGETS unvol
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
