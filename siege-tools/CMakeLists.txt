cmake_minimum_required(VERSION 3.20)
project(siege-tools)

add_subdirectory(unvol)
add_subdirectory(nuvol)

add_subdirectory(dts-to-json)
add_subdirectory(dts-to-obj)

# TODO fix compile issues with json library
#add_subdirectory(json-to-dts)
