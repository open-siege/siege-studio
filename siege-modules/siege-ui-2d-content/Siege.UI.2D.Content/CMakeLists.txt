cmake_minimum_required(VERSION 3.20)
project(Siege.UI.TwoD.Content VERSION 1.0.0.0 LANGUAGES CSharp)

include(CSharpUtilities)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/src/AssemblyInfo.cs.template" "${CMAKE_BINARY_DIR}/Config/${PROJECT_NAME}/AssemblyInfo.cs")

file(GLOB_RECURSE SRC_FILES
        src/*.cs)

add_library(${PROJECT_NAME} SHARED "${CMAKE_BINARY_DIR}/Config/${PROJECT_NAME}/AssemblyInfo.cs" ${SRC_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DOTNET_REFERENCES "System;System.Core;System.Runtime;System.Runtime.InteropServices")
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-content)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)