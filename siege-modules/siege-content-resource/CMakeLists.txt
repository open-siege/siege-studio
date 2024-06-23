cmake_minimum_required(VERSION 3.20)
project(siege-content-resource)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/module.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-resource)
else()
    add_library(${PROJECT_NAME} SHARED src/views/vol_controller.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-resource)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)

add_custom_target(Siege.Resource.Content
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/Siege.Resource.Content.dll
        DEPENDS Siege.Platform siege-content-resource
)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/Siege.Resource.Content.dll CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)