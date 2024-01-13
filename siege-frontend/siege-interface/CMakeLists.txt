cmake_minimum_required(VERSION 3.20)
project(siege-interface)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(imgui REQUIRED)
find_package(SDL2 REQUIRED)

if(WIN32)
    add_library(siege-interface bindings/imgui_impl_sdl.cpp
            bindings/imgui_impl_dx11.cpp
            src/renderer/windows.cpp
            src/interface/main.cpp)
    target_link_libraries(siege-interface PRIVATE d3d11)
else()
    add_library(siege-interface bindings/imgui_impl_sdl.cpp
            bindings/imgui_impl_sdlrenderer.cpp
            src/renderer/sdl.cpp
            src/interface/main.cpp)
endif()

set_property(TARGET siege-interface PROPERTY CXX_STANDARD 23)
target_include_directories(siege-interface PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bindings ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(siege-interface PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-interface PUBLIC SDL2::SDL2-static imgui::imgui)

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS siege-interface CONFIGURATIONS Debug Release RUNTIME DESTINATION lib)
