cmake_minimum_required(VERSION 3.30)
project(siege-tools)

add_subdirectory(unvol)

add_subdirectory(dts-convert)

if(WIN32) 
	add_subdirectory(game-unpack)
endif()

# TODO fix compile issues with json library
#add_subdirectory(json-to-dts)
