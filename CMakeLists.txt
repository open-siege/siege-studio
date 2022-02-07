project(open-siege)
cmake_minimum_required(VERSION 3.17)

add_subdirectory(3space)
add_library(3space::3space ALIAS 3space)

add_subdirectory(dts-to-json)
add_subdirectory(json-to-dts)
add_subdirectory(dts-to-obj)
add_subdirectory(unvol)
add_subdirectory(siege-shell)
add_subdirectory(3space-studio)

# TODO fix mdl-to-json as it is vastly out of date
#add_subdirectory(mdl-to-json)
