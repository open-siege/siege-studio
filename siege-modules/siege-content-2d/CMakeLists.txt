cmake_minimum_required(VERSION 3.20)
project(siege-content-2d)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/module.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-content uxtheme)
else()
    add_library(${PROJECT_NAME} SHARED src/views/bmp_controller.cpp src/views/pal_controller.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-platform siege-content)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)


set(OUTPUT_DIR $<$<CONFIG:Debug>:Debug> $<$<CONFIG:RelWithDebInfo>:Debug> $<$<CONFIG:Release>:Release> $<$<CONFIG:MinSizeRel>:Release>)

add_custom_target(Siege.2D.Content
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/Siege.2D.Content.dll
        DEPENDS Siege.Platform siege-content-2d
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Siege.2D.Content.dll CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)