cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(siege-modules)
add_subdirectory(siege-tools)
add_subdirectory(siege-studio)

get_target_property(NUVOL_BINARY_DIR nuvol BINARY_DIR)

add_custom_target(siege-studio-dependencies ALL COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:siege-studio>)
add_custom_command(TARGET siege-studio-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-2d-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-3d-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-resource-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-audio-content> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unvol> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dts-to-json> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dts-to-obj> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${NUVOL_BINARY_DIR} $<TARGET_FILE_DIR:siege-studio>)

add_dependencies(siege-studio-dependencies siege-2d-content siege-3d-content siege-resource-content siege-audio-content nuvol)
add_dependencies(siege-studio siege-studio-dependencies)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-launcher)
    add_subdirectory(siege-launcher)
    add_custom_target(siege-launcher-dependencies ALL COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:siege-launcher>)
    add_dependencies(siege-launcher-dependencies siege-studio-dependencies siege-extension siege-extension-starsiege)
    add_dependencies(siege-launcher siege-launcher-dependencies)

    add_custom_command(TARGET siege-launcher-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-extension> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-extension-starsiege> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-2d-content> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-3d-content> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-resource-content> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-audio-content> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-game-content> $<TARGET_FILE_DIR:siege-launcher>)
endif()