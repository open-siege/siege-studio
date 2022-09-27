cmake_minimum_required(VERSION 3.16)
project(siege-input)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(imgui REQUIRED)

if(WIN32)
    add_executable(siege-input-config bindings/imgui_impl_sdl.cpp
            bindings/imgui_impl_dx11.cpp
            src/renderer/windows.cpp
            src/main.cpp)
else()
    add_executable(siege-input-config bindings/imgui_impl_sdl.cpp
            bindings/imgui_impl_sdlrenderer.cpp
            src/renderer/sdl.cpp
            src/main.cpp)
endif()

set_property(TARGET siege-input-config PROPERTY CXX_STANDARD 17)
target_include_directories(siege-input-config PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/bindings ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-input-config PRIVATE imgui::imgui siege-input)

if(WIN32)
    target_link_libraries(siege-input-config PRIVATE d3d11)
endif()

install(TARGETS siege-input-config CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
