cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

add_subdirectory(siege-installer)
add_subdirectory(siege-input)
add_subdirectory(siege-interface)
add_subdirectory(siege-launcher)
add_subdirectory(siege-studio)

if (WIN32)
    add_subdirectory(siege-modules)
    add_subdirectory(siege-launcher-win32)

    add_custom_target(copy-when-done ALL COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-bmp> $<TARGET_FILE_DIR:siege-launcher-win32>)
    add_custom_command(TARGET copy-when-done POST_BUILD 
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-sfx> $<TARGET_FILE_DIR:siege-launcher-win32>
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-dts> $<TARGET_FILE_DIR:siege-launcher-win32>
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-vol> $<TARGET_FILE_DIR:siege-launcher-win32>)

    add_dependencies(copy-when-done siege-bmp siege-launcher-win32)
endif()