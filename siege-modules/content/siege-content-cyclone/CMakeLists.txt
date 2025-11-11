cmake_minimum_required(VERSION 3.30)
project(siege-content-cyclone)

add_library(${PROJECT_NAME}-resource STATIC src/cyclone_resource.cpp)
set_target_properties(${PROJECT_NAME}-resource PROPERTIES CXX_STANDARD 23 POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME}-resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME}-resource PUBLIC siege-content-common)
install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}-resource
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)
