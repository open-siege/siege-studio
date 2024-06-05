cmake_minimum_required(VERSION 3.20)
project(siege-modules)


add_subdirectory(siege-platform)
add_subdirectory(siege-resource)
add_subdirectory(siege-content)

# for both siege-studio and siege-launcher
add_subdirectory(siege-content-2d)
add_subdirectory(siege-content-3d)
add_subdirectory(siege-content-resource)
add_subdirectory(siege-content-audio)

# the public parts of siege-launcher
add_subdirectory(siege-configuration)
add_subdirectory(siege-extension)
add_subdirectory(siege-extension-darkstar)
add_subdirectory(siege-extension-id-tech)
add_subdirectory(siege-installation)