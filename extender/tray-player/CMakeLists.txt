cmake_minimum_required(VERSION 3.16)
project(darkstar-extender-music-player)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(wxwidgets REQUIRED)
find_package(sfml REQUIRED)
find_package(mio REQUIRED)

add_library(music-player STATIC src/music_player.cpp)
set_property(TARGET music-player PROPERTY CXX_STANDARD 17)
target_link_libraries(music-player PRIVATE mio::mio-headers SFML::main SFML::system SFML::audio)

add_executable(tray-player src/main.cpp)
set_property(TARGET tray-player PROPERTY CXX_STANDARD 17)
target_link_libraries(tray-player PRIVATE wxwidgets::wxwidgets mio::mio-headers)

add_executable(music-player-tester src/test.cpp)
set_property(TARGET music-player-tester PROPERTY CXX_STANDARD 17)
target_link_libraries(music-player-tester PRIVATE wxwidgets::wxwidgets music-player)

if(MSVC)
    target_compile_definitions(tray-player PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS tray-player CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
