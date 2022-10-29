project(open-siege-tools)
cmake_minimum_required(VERSION 3.17)

add_subdirectory(dts-to-json)
add_subdirectory(json-to-dts)
add_subdirectory(dts-to-obj)
add_subdirectory(unvol)
add_subdirectory(legacy)

# TODO fix mdl-to-json as it is vastly out of date
#add_subdirectory(mdl-to-json)
