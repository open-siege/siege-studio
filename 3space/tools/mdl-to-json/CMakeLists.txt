project(mdl-to-json)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.

add_executable(mdl-to-json src/convert_mdl.cpp)
set_property(TARGET mdl-to-json PROPERTY CXX_STANDARD 23)
target_link_libraries(mdl-to-json 3space::3space)

install(TARGETS mdl-to-json
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
