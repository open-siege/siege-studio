cmake_minimum_required(VERSION 3.16)
project(siege-frontend)

add_subdirectory(siege-installer)
add_subdirectory(siege-input)
add_subdirectory(siege-launcher)

# TODO siege-studio is going to need a lot of work to be functional again
# because of this project dropping wxwidgets and sfml
#add_subdirectory(siege-studio)
