cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(siege-modules)
#add_subdirectory(siege-studio)
#add_subdirectory(siege-launcher)

#add_custom_target(copy-when-done ALL COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ui-2d-content> $<TARGET_FILE_DIR:siege-studio>)
#add_custom_command(TARGET copy-when-done POST_BUILD 
          #  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-ui-3d-content> $<TARGET_FILE_DIR:siege-studio>
          #  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-ui-resource-content> $<TARGET_FILE_DIR:siege-studio>
          #  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-mfc> $<TARGET_FILE_DIR:siege-studio>)

#add_dependencies(copy-when-done siege-ui-2d-content siege-ui-3d-content siege-ui-resource-content siege-ui-audio-content)
#add_dependencies(siege-studio copy-when-done)