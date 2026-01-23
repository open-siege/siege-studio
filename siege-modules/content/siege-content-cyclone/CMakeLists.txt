cmake_minimum_required(VERSION 3.30)
project(siege-content-cyclone)

add_library(${PROJECT_NAME}-resource STATIC src/cyclone_resource.cpp)
add_library(${PROJECT_NAME}-scripting STATIC src/uprising.cpp)

set(PROJECT_TARGETS ${PROJECT_NAME}-scripting ${PROJECT_NAME}-resource)

foreach(TARGET IN ITEMS ${PROJECT_TARGETS})
    set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${TARGET} PUBLIC siege-content-common)
    target_include_directories(${TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endforeach()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}-resource
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)
