cmake_minimum_required(VERSION 3.20)
project(3d-tools)

add_subdirectory(dts-to-json)
add_subdirectory(dts-to-obj)

# TODO fix compile issues with json library
#add_subdirectory(json-to-dts)