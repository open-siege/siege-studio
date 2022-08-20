cmake_minimum_required(VERSION 3.16)
project(darkstar-extender-music-player)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(sfml REQUIRED)

add_library(music-player STATIC src/music_player.cpp)
set_property(TARGET music-player PROPERTY CXX_STANDARD 17)
target_include_directories(music-player PUBLIC include)
target_link_libraries(music-player PRIVATE SFML::main SFML::system SFML::audio)

