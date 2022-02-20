cmake_minimum_required(VERSION 3.16)
project(darkstar-extender-music-player)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(wxwidgets REQUIRED)

add_executable(tray-player src/main.cpp)
set_property(TARGET tray-player PROPERTY CXX_STANDARD 17)
target_link_libraries(tray-player PRIVATE wxwidgets::wxwidgets)

if(MSVC)
    target_compile_definitions(tray-player PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS tray-player CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
