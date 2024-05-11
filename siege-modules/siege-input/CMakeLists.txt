cmake_minimum_required(VERSION 3.20)
project(siege-input)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(SDL2 REQUIRED)

if(WIN32)
    add_library(${PROJECT_NAME} STATIC 
    src/platform/common.cpp 
    src/platform/windows.cpp src/config.cpp 
    src/virtual_joystick.cpp 
    src/profiles/games/game_info.cpp 
    src/profiles/games/id_tech_2.actions.cpp
    src/profiles/games/id_tech_2.meta.cpp
    src/profiles/games/dosbox.cpp
    src/profiles/games/racer.cpp
    src/profiles/games/krass.cpp
    src/profiles/games/serious.cpp
    src/profiles/games/unreal.cpp) 
else()
    add_library(${PROJECT_NAME} STATIC 
    src/platform/common.cpp 
    src/platform/other.cpp 
    src/config.cpp 
    src/virtual_joystick.cpp
    src/profiles/games/game_info.cpp
    src/profiles/games/id_tech_2.meta.cpp
    src/profiles/games/id_tech_2.actions.cpp
    src/profiles/games/dosbox.cpp
    src/profiles/games/racer.cpp
    src/profiles/games/krass.cpp
    src/profiles/games/serious.cpp
    src/profiles/games/unreal.cpp)
endif()

add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/profiles $<TARGET_FILE_DIR:${PROJECT_NAME}>/
        )

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2-static nlohmann_json::nlohmann_json siege-configuration)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dinput8)
endif()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION lib)
