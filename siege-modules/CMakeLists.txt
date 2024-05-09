cmake_minimum_required(VERSION 3.20)
project(siege-modules)


add_subdirectory(siege-platform)
add_subdirectory(siege-resource)
add_subdirectory(siege-content)

# for both siege-studio and siege-launcher
add_subdirectory(siege-2d-content)
add_subdirectory(siege-3d-content)
add_subdirectory(siege-resource-content)
add_subdirectory(siege-audio-content)

# for siege-launcher only (which is private code)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-configuration)
    add_subdirectory(siege-configuration)
endif()

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-installation)
    add_subdirectory(siege-installation)
endif()

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-game-input)
    add_subdirectory(siege-game-input)
endif()



