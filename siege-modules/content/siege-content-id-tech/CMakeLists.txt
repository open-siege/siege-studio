cmake_minimum_required(VERSION 3.30)
project(siege-content-id-tech)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

add_library(${PROJECT_NAME}-2d STATIC src/m8.cpp)
add_library(${PROJECT_NAME}-3d STATIC src/mdl.cpp)
target_link_libraries(${PROJECT_NAME}-3d PRIVATE ${PROJECT_NAME}-2d)
add_library(${PROJECT_NAME}-scripting STATIC src/id_tech_2)
add_library(${PROJECT_NAME}-resource STATIC src/pak_resource.cpp src/wad_resource.cpp)
target_link_libraries(${PROJECT_NAME}-resource PRIVATE ${PROJECT_NAME}-2d ${PROJECT_NAME}-3d ${PROJECT_NAME}-scripting)

set(PROJECT_TARGETS ${PROJECT_NAME}-2d ${PROJECT_NAME}-3d ${PROJECT_NAME}-scripting ${PROJECT_NAME}-resource)

foreach(TARGET IN ITEMS ${PROJECT_TARGETS})
    set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${TARGET} PUBLIC siege-content-common)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endforeach()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_TARGETS}
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