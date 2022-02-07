project(lib3space)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(glm REQUIRED)
find_package(span-lite REQUIRED)
find_package(Catch2 REQUIRED)
find_package(pegtl REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

file(GLOB LIB_SRC_FILES
        src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp)

list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})

file(GLOB TESTABLE_SRC_FILES src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp)

add_library(3space STATIC ${LIB_SRC_FILES})
set_property(TARGET 3space PROPERTY CXX_STANDARD 17)
target_include_directories(3space PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(3space PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(3space nlohmann_json::nlohmann_json Boost::Boost glm::glm nonstd::span-lite taocpp::pegtl)

add_executable(3space-tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
set_property(TARGET 3space-tests PROPERTY CXX_STANDARD 17)
target_include_directories(3space-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(3space-tests PRIVATE Catch2::Catch2 nlohmann_json::nlohmann_json Boost::Boost glm::glm nonstd::span-lite taocpp::pegtl)

include(CTest)
include(Catch)
catch_discover_tests(3space-tests)

file(GLOB_RECURSE HEADER_FILES src/*.hpp)
add_custom_command(TARGET 3space
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

foreach(HEADER_FILE ${HEADER_FILES})
    file(RELATIVE_PATH HEADER_FILE_REL ${CMAKE_CURRENT_SOURCE_DIR}/src ${HEADER_FILE})
    add_custom_command(TARGET 3space
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${HEADER_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/include/${HEADER_FILE_REL}
            )
endforeach()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS 3space
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION lib)


