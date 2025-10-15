cmake_minimum_required(VERSION 3.30)
project(siege-presentation-executable)

find_package(nlohmann_json REQUIRED)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/${PROJECT_NAME}.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-extension hid Imagehlp nlohmann_json::nlohmann_json)
else()
    add_library(${PROJECT_NAME} SHARED src/empty.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-platform)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)


install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)