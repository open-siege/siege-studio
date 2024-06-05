cmake_minimum_required(VERSION 3.20)
project(game-unpack)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(cpr REQUIRED)

add_executable(${PROJECT_NAME} src/game-unpack.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr siege-installation siege-resource)
target_include_directories(${PROJECT_NAME} PRIVATE src)

## Install/Packaging Section
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

