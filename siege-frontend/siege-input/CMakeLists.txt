cmake_minimum_required(VERSION 3.16)
project(siege-input)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(SDL2 REQUIRED)

if(WIN32)
    add_library(siege-input STATIC src/platform/common.cpp src/platform/windows.cpp src/config.cpp src/virtual_joystick.cpp)
else()
    add_library(siege-input STATIC src/platform/common.cpp src/platform/other.cpp src/config.cpp src/virtual_joystick.cpp)
endif()

add_custom_command(TARGET siege-input
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles $<TARGET_FILE_DIR:siege-input>/
        )

set_property(TARGET siege-input PROPERTY CXX_STANDARD 17)
target_include_directories(siege-input PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(siege-input PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-input PUBLIC SDL2::SDL2-static nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(siege-input PRIVATE dinput8)
endif()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS siege-input CONFIGURATIONS Debug Release RUNTIME DESTINATION lib)
