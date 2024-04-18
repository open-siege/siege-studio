cmake_minimum_required(VERSION 3.20)
project(siege-modules)


add_subdirectory(siege-platform)
add_subdirectory(siege-resource)
add_subdirectory(siege-content)
add_subdirectory(siege-configuration)

# for siege-studio and siege-launcher
add_subdirectory(siege-ui-2d-content)
add_subdirectory(siege-ui-3d-content)
add_subdirectory(siege-ui-resource-content)
add_subdirectory(siege-ui-audio-content)

# for siege-launcher only
add_subdirectory(siege-ui-game-input)
add_subdirectory(siege-ui-game-installer)

