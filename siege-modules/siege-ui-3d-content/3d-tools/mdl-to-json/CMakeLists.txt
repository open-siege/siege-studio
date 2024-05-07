project(mdl-to-json)
cmake_minimum_required(VERSION 3.20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_executable(mdl-to-json src/convert_mdl.cpp)
set_property(TARGET mdl-to-json PROPERTY CXX_STANDARD 23)
target_link_libraries(mdl-to-json siege-content)

if (MSVC)
        set_property(TARGET mdl-to-json PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

install(TARGETS mdl-to-json
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
