cmake_minimum_required(VERSION 3.30)
project(game-unpack)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
    add_executable(${PROJECT_NAME} src/game-unpack.win32.cpp "src/resource_maker.cpp")
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop)
else()
    # TODO make extension and installation modules work in a cross platform way
    add_executable(${PROJECT_NAME} src/game-unpack.cpp)
endif()
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} PRIVATE siege-content-common)
target_include_directories(${PROJECT_NAME} PRIVATE src)

include(copy_deps.cmake)

## Install/Packaging Section
install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
