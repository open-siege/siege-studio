cmake_minimum_required(VERSION 3.30)
project(siege-tools)

add_subdirectory(unvol)

add_subdirectory(dts-to-json)
add_subdirectory(dts-to-obj)

add_subdirectory(game-unpack)

# TODO fix compile issues with json library
#add_subdirectory(json-to-dts)
