project(siege-shell)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(wxwidgets REQUIRED)

add_executable(siege-shell src/main.cpp)
set_property(TARGET siege-shell PROPERTY CXX_STANDARD 17)
target_link_libraries(siege-shell wxwidgets::wxwidgets)
add_custom_command(TARGET siege-shell
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/besieged-theme.png $<TARGET_FILE_DIR:siege-shell>/besieged-theme.png
        )

install(TARGETS siege-shell
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

install(FILES src/besieged-theme.png DESTINATION bin)
