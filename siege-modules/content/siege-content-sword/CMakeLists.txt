cmake_minimum_required(VERSION 3.30)
project(siege-content-sword)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE WIN32_SRC_FILES src/*.win32.cpp)
file(GLOB LIB_SRC_FILES src/*.cpp src/**/*.cpp)

list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES} ${WIN32_SRC_FILES})

add_library(${PROJECT_NAME} STATIC ${LIB_SRC_FILES})

if (WIN32)
    target_sources(${PROJECT_NAME} PRIVATE ${WIN32_SRC_FILES})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} siege-content-common)
install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)