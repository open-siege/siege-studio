cmake_minimum_required(VERSION 3.17)
project(lib3space)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(span-lite REQUIRED)
find_package(Catch2 REQUIRED)
find_package(pegtl REQUIRED)
find_package(libzip REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

file(GLOB LIB_SRC_FILES
        src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp)

list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})

file(GLOB TESTABLE_SRC_FILES src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp
        src/configurations/*.cpp)

add_library(3space STATIC ${LIB_SRC_FILES})
set_property(TARGET 3space PROPERTY CXX_STANDARD 17)
target_include_directories(3space PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3space
                        nlohmann_json::nlohmann_json 
                        Boost::boost
                        glm::glm 
                        nonstd::span-lite 
                        taocpp::pegtl
                        libzip::zip)

add_executable(3space-tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
set_property(TARGET 3space-tests PROPERTY CXX_STANDARD 17)
target_include_directories(3space-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(3space-tests PRIVATE Catch2::Catch2WithMain 
                        nlohmann_json::nlohmann_json 
                        Boost::boost 
                        glm::glm 
                        nonstd::span-lite 
                        taocpp::pegtl 
                        libzip::zip)

add_library(3space::3space ALIAS 3space)
add_subdirectory(tools)
include(CTest)
include(Catch)
catch_discover_tests(3space-tests)

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS 3space
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)


