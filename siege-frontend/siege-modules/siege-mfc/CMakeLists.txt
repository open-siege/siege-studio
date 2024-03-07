cmake_minimum_required(VERSION 3.20)
project(siege-mfc)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

set(CMAKE_MFC_FLAG 2)
add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
target_compile_definitions(${PROJECT_NAME} PRIVATE _AFXDLL)
target_compile_definitions(${PROJECT_NAME} PRIVATE VC_EXTRALEAN)
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC comctl32)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)