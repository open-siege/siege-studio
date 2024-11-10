cmake_minimum_required(VERSION 3.28)
project(siege-installation)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


add_library(${PROJECT_NAME} STATIC src/games/games.cpp)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

## Install/Packaging Section
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

