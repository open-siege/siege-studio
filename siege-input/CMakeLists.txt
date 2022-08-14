cmake_minimum_required(VERSION 3.16)
project(siege-input)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(imgui REQUIRED)
find_package(SDL2 REQUIRED)

if(WIN32)
    add_executable(siege-input bindings/imgui_impl_sdl.cpp bindings/imgui_impl_sdlrenderer.cpp src/common.cpp src/windows.cpp src/main.cpp)
else()
    add_executable(siege-input bindings/imgui_impl_sdl.cpp bindings/imgui_impl_sdlrenderer.cpp src/common.cpp src/other.cpp src/main.cpp)
endif()


set_property(TARGET siege-input PROPERTY CXX_STANDARD 17)
target_include_directories(siege-input PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bindings ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(siege-input PRIVATE imgui::imgui SDL2::SDL2-static nlohmann_json::nlohmann_json)

if(WIN32)
    target_link_libraries(siege-input PRIVATE dinput8)
endif()

install(TARGETS siege-input CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
