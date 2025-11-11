cmake_minimum_required(VERSION 3.30)
project(siege-content-mw2)

add_library(${PROJECT_NAME}-3d STATIC src/bwd.cpp src/wtb.cpp src/wtb_renderable_shape.cpp)
add_library(${PROJECT_NAME}-resource STATIC src/prj_resource.cpp)
target_link_libraries(${PROJECT_NAME}-resource PRIVATE ${PROJECT_NAME}-3d)

set(PROJECT_TARGETS ${PROJECT_NAME}-3d ${PROJECT_NAME}-resource)

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