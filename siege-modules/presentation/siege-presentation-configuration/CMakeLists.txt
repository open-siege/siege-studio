cmake_minimum_required(VERSION 3.30)
project(siege-presentation-configuration)

file(GLOB_RECURSE LIB_SRC_FILES src/*.cpp)
file(GLOB_RECURSE WIN32_SRC_FILES src/*.win32.cpp)     
list(REMOVE_ITEM LIB_SRC_FILES ${WIN32_SRC_FILES})

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${LIB_SRC_FILES} ${WIN32_SRC_FILES} src/${PROJECT_NAME}.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-configuration)
else()
    add_library(${PROJECT_NAME} SHARED ${LIB_SRC_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-configuration)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)


install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)