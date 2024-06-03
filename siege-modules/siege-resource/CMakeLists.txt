cmake_minimum_required(VERSION 3.20)
project(siege-resource)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Catch2 REQUIRED)
find_package(libzip REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)
file(GLOB LIB_SRC_FILES src/*.cpp src/**/*.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})
file(GLOB TESTABLE_SRC_FILES src/*.cpp src/**/*.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} libzip::zip siege-platform)

add_executable(${PROJECT_NAME}-tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME}-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain 
                        siege-platform
                        libzip::zip)
include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}-tests)

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)

add_custom_command(
     TARGET ${PROJECT_NAME}-tests
     COMMENT "Running ${PROJECT_NAME}-tests"
     POST_BUILD 
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failures
)


