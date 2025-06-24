project(unvol)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
    add_executable(unvol src/unvol.cpp src/unvol.win32.cpp src/unvol.rc)
else()
    add_executable(unvol src/unvol.cpp src/unvol.other.cpp)
endif()


set_property(TARGET unvol PROPERTY CXX_STANDARD 23)
target_link_libraries(unvol siege-resource)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(unvol TBB::tbb)
endif()

install(TARGETS unvol
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
