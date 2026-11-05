cmake_minimum_required(VERSION 3.30)
project(siege-presentation-executable)

find_package(nlohmann_json REQUIRED)
find_package(Catch2 REQUIRED)

file(GLOB_RECURSE SRC_FILES
        src/*.cpp)
file(GLOB_RECURSE TEST_SRC_FILES
        src/*.test.cpp)

list(REMOVE_ITEM SRC_FILES ${TEST_SRC_FILES})

if (WIN32)
    add_library(${PROJECT_NAME} SHARED ${SRC_FILES} src/${PROJECT_NAME}.def)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop siege-extension hid Imagehlp nlohmann_json::nlohmann_json)
else()
    add_library(${PROJECT_NAME} SHARED src/empty.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-platform)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_SCAN_FOR_MODULES ON
    POSITION_INDEPENDENT_CODE ON)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)


install(TARGETS ${PROJECT_NAME} CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)

if (WIN32)
    add_executable(${PROJECT_NAME}-tests
        src/views/config_gen.test.cpp
        src/views/config_gen.cpp
        src/views/exe_shared.cpp
        src/views/input.cpp
        src/views/input_xbox.cpp
        src/views/input_other.cpp
        src/views/input_playstation.cpp
        src/views/input_custom.cpp)
    set_target_properties(${PROJECT_NAME}-tests PROPERTIES CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON)
    target_include_directories(${PROJECT_NAME}-tests PRIVATE ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(${PROJECT_NAME}-tests PRIVATE
        Catch2::Catch2WithMain
        siege-win-desktop
        siege-extension
        hid
        Imagehlp
        nlohmann_json::nlohmann_json)

    if (MSVC)
        target_compile_options(${PROJECT_NAME}-tests PRIVATE /bigobj)
    endif()

    include(CTest)
    include(Catch)
    catch_discover_tests(${PROJECT_NAME}-tests)
endif()
