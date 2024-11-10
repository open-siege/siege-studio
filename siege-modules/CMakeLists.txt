cmake_minimum_required(VERSION 3.28)
project(siege-modules)


add_subdirectory(siege-platform)
add_subdirectory(siege-resource)
add_subdirectory(siege-content)

# for both siege-studio and siege-launcher
add_subdirectory(siege-content-2d)
add_subdirectory(siege-content-3d)
add_subdirectory(siege-content-resource)
add_subdirectory(siege-content-audio)
add_subdirectory(siege-content-configuration)
add_subdirectory(siege-content-executable)

# the public parts of siege-launcher
add_subdirectory(siege-configuration)
add_subdirectory(siege-extension)
add_subdirectory(siege-extension-3space)
add_subdirectory(siege-extension-id-tech)
add_subdirectory(siege-extension-unreal)
add_subdirectory(siege-extension-cry-engine)
add_subdirectory(siege-extension-krass)
add_subdirectory(siege-extension-other)
add_subdirectory(siege-extension-real-space)
add_subdirectory(siege-extension-serious)
add_subdirectory(siege-extension-isi-motor)
add_subdirectory(siege-installation)