cmake_minimum_required(VERSION 3.20)
project(siege-win32)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
#target_precompile_headers(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include/win32_messages.hpp ${PROJECT_SOURCE_DIR}/include/win32_controls.hpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP NOMINMAX COM_NO_WINDOWS_H)

target_link_libraries(${PROJECT_NAME} PUBLIC comctl32 oleacc siege-games-win)

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)