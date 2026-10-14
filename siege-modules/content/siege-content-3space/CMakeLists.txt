cmake_minimum_required(VERSION 3.30)
project(siege-content-3space)

find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

add_library(${PROJECT_NAME}-2d STATIC src/bmp/bitmap.cpp src/pal/palette.cpp src/fnt/font.cpp)

add_library(${PROJECT_NAME}-3d STATIC src/dts/dts_renderable_shape.cpp 
                                      src/dts/3space.cpp 
                                      src/dts/3space_v2.cpp
                                      src/dts/3space_renderable_shape.cpp 
                                      src/dts/darkstar.cpp
                                      src/dts/torque.cpp)

target_link_libraries(${PROJECT_NAME}-3d PRIVATE ${PROJECT_NAME}-2d)

add_library(${PROJECT_NAME}-scripting STATIC src/mis/mission.cpp src/gui/interface.cpp src/3space.cpp)

add_library(${PROJECT_NAME}-resource STATIC src/clm_resource.cpp 
                                            src/darkstar_resource.cpp 
                                            src/three_space_resource.cpp
                                            src/trophy_bass_resource.cpp
)

target_link_libraries(${PROJECT_NAME}-resource PRIVATE ${PROJECT_NAME}-2d ${PROJECT_NAME}-3d ${PROJECT_NAME}-scripting)

set(PROJECT_TARGETS ${PROJECT_NAME}-2d ${PROJECT_NAME}-3d ${PROJECT_NAME}-scripting ${PROJECT_NAME}-resource)

foreach(TARGET IN ITEMS ${PROJECT_TARGETS})
    set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
        POSITION_INDEPENDENT_CODE ON)
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

# test code — link content libs (modules) instead of recompiling all sources
add_executable(${PROJECT_NAME}-tests ${TEST_SRC_FILES})
set_target_properties(${PROJECT_NAME}-tests PROPERTIES CXX_STANDARD 23 CXX_SCAN_FOR_MODULES ON)
target_include_directories(${PROJECT_NAME}-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain ${PROJECT_TARGETS})

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
