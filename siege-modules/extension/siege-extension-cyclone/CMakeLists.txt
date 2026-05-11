cmake_minimum_required(VERSION 3.30)
project(siege-extension-cyclone)

if (WIN32)
    add_library(siege-extension-uprising SHARED src/uprising.cpp src/uprising.def)
    set_target_properties(siege-extension-uprising PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-uprising PRIVATE siege-extension siege-content-cyclone-scripting)

    add_library(siege-extension-uprising-2 SHARED src/uprising-2.cpp src/uprising-2.def)
    set_target_properties(siege-extension-uprising-2 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-uprising-2 PRIVATE siege-extension siege-content-cyclone-scripting)
endif()
