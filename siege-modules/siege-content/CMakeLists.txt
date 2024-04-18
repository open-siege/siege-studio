cmake_minimum_required(VERSION 3.20)
project(siege-content)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2 REQUIRED)
find_package(pegtl REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)
file(GLOB LIB_SRC_FILES src/*.cpp src/**/*.cpp)
list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})
file(GLOB TESTABLE_SRC_FILES src/*.cpp src/**/*.cpp)

add_library(${PROJECT_NAME} STATIC ${LIB_SRC_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} nlohmann_json::nlohmann_json glm::glm taocpp::pegtl siege-platform)

if (MSVC)
        set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# TODO fix linker error with tests on Windows
if (WIN32)
        add_executable(${PROJECT_NAME}-tests ${CMAKE_CURRENT_SOURCE_DIR}/src/empty.cpp)
        target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain)        
else()
        add_executable(${PROJECT_NAME}-tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
        set_property(TARGET ${PROJECT_NAME}-tests PROPERTY CXX_STANDARD 23)
        target_include_directories(${PROJECT_NAME}-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_link_libraries(${PROJECT_NAME}-tests PRIVATE Catch2::Catch2WithMain 
                                nlohmann_json::nlohmann_json  
                                glm::glm  
                                taocpp::pegtl 
                                libzip::zip)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME}-tests)

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)


