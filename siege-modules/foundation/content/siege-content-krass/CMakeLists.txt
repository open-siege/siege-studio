cmake_minimum_required(VERSION 3.30)
project(siege-content-krass)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)
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

# test code
file(GLOB TESTABLE_SRC_FILES src/*.cpp src/**/*.cpp)
if (NOT WIN32)
        list(REMOVE_ITEM TESTABLE_SRC_FILES ${TEST_SRC_FILES} ${WIN32_SRC_FILES})
endif()
add_executable(${PROJECT_NAME}-tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
set_property(TARGET ${PROJECT_NAME}-tests PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME}-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain siege-content-common)

if (MSVC)
        target_compile_options(${PROJECT_NAME}-tests PRIVATE /bigobj)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}-tests)

add_custom_command(
     TARGET ${PROJECT_NAME}-tests
     COMMENT "Running ${PROJECT_NAME}-tests"
     POST_BUILD 
     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
     COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failure
)