cmake_minimum_required(VERSION 3.20)
project(siege-mfc)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp
        src/*.rc)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE VC_EXTRALEAN _ATL_CSTRING_EXPLICIT_CONSTRUCTORS _AFX_NO_DAO_SUPPORT)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)        

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)