project(siege-shell)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(wxwidgets REQUIRED)
find_package(nanosvg REQUIRED)

add_executable(siege-shell src/main.cpp)
set_property(TARGET siege-shell PROPERTY CXX_STANDARD 17)
target_link_libraries(siege-shell wxwidgets::wxwidgets nanosvg::nanosvg)

add_custom_command(TARGET siege-shell
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/besieged-theme.svg $<TARGET_FILE_DIR:siege-shell>/besieged-theme.svg
        )

if(MSVC)
    target_compile_definitions(siege-shell PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS siege-shell
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

install(FILES src/besieged-theme.svg DESTINATION bin)
