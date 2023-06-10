project(unvol)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(unvol src/unvol.cpp)
set_property(TARGET unvol PROPERTY CXX_STANDARD 17)
target_link_libraries(unvol 3space::3space)

install(TARGETS unvol
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
