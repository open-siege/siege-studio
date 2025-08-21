cmake_minimum_required(VERSION 3.30)
project(siege-content-all)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)
file(GLOB LIB_SRC_FILES src/*.cpp src/**/*.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})
file(GLOB TESTABLE_SRC_FILES src/*.cpp src/**/*.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} siege-content-id-tech siege-content-3space siege-content-mw2 siege-content-mw4 siege-content-sword siege-content-cyclone siege-content-colony-wars)

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)


