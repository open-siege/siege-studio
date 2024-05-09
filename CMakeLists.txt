cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(siege-modules)
add_subdirectory(siege-studio)

add_custom_target(copy-when-done ALL COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-2d-content> $<TARGET_FILE_DIR:siege-studio>)
add_custom_command(TARGET copy-when-done POST_BUILD 
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-3d-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-resource-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-win-mfc> $<TARGET_FILE_DIR:siege-studio>)

add_dependencies(copy-when-done siege-2d-content siege-3d-content siege-resource-content siege-audio-content)
add_dependencies(siege-studio copy-when-done)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-launcher)
    add_subdirectory(siege-launcher)
endif()