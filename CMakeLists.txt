cmake_minimum_required(VERSION 3.16)
project(3SpaceStudio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(boost_endian REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(wxwidgets REQUIRED)
find_package(glm REQUIRED)
find_package(Catch2 REQUIRED)

file(GLOB TEST_SRC_FILES src/*.test.cpp src/**/*.test.cpp src/**/**/*.test.cpp)

file(GLOB DTS_SRC_FILES src/content/*.cpp
        src/content/dts/*.cpp
        src/dts-to-json/*.cpp)

file(GLOB OBJ_SRC_FILES src/content/*.cpp
        src/content/dts/*.cpp
        src/dts-to-obj/*.cpp)

file(GLOB JSON_SRC_FILES
        src/content/*.cpp
        src/content/dts/*.cpp
        src/json-to-dts/*.cpp)
file(GLOB VOL_SRC_FILES src/resource/*.cpp src/unvol/*.cpp)
file(GLOB MIS_SRC_FILES src/mis-to-json/*.cpp)
file(GLOB DTS_VIEWER_SRC_FILES
        src/*.cpp
        src/content/*.cpp
        src/content/**/*.cpp
        src/resource/*.cpp
        src/3space-studio/views/*.cpp src/3space-studio/*.cpp)

list(REMOVE_ITEM DTS_VIEWER_SRC_FILES ${TEST_SRC_FILES})

file(GLOB TESTABLE_SRC_FILES src/content/*.cpp
        src/content/**/*.cpp
        src/resource/*.cpp)

add_executable(dts-to-json ${DTS_SRC_FILES})
add_executable(dts-to-obj ${OBJ_SRC_FILES})
add_executable(json-to-dts ${JSON_SRC_FILES})
add_executable(unvol ${VOL_SRC_FILES})
add_executable(3space-studio ${DTS_VIEWER_SRC_FILES})

include_directories(packages/include)
include_directories(src)

foreach(target ${boost_endian_LIBRARIES_TARGETS})
    get_target_property(TARGET_INCLUDES ${target} INTERFACE_INCLUDE_DIRECTORIES)
    set(BOOST_INCLUDES ${BOOST_INCLUDES} ${TARGET_INCLUDES})
endforeach()

set(BASIC_INCLUDES ${BOOST_INCLUDES} ${boost_endian_INCLUDE_DIRS} ${nlohmann_json_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${Catch2_INCLUDE_DIRS})
set(GUI_INCLUDES ${BASIC_INCLUDES} ${ImGui-SFML_INCLUDE_DIRS} ${wxwidgets_INCLUDE_DIRS})
set(GUI_LIBS ${ImGui-SFML_LIBRARIES} ${wxwidgets_LIBRARIES} ${glm_LIBRARIES})

target_include_directories(dts-to-json PRIVATE ${BASIC_INCLUDES})
target_include_directories(dts-to-obj PRIVATE ${BASIC_INCLUDES})
target_include_directories(json-to-dts PRIVATE ${BASIC_INCLUDES})

target_include_directories(unvol PRIVATE ${GUI_INCLUDES})

target_include_directories(3space-studio PRIVATE ${GUI_INCLUDES})
target_link_libraries(3space-studio PRIVATE ${GUI_LIBS})

if(MSVC)
    target_compile_options(dts-to-json PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(dts-to-obj PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(json-to-dts PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(unvol PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:/O2>)
else()
    target_compile_options(dts-to-json PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(dts-to-obj PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(json-to-dts PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(unvol PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:-O3>)
endif()

add_executable(tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
target_include_directories(tests PRIVATE ${Catch2_INCLUDES} ${GUI_INCLUDES})
target_link_libraries(tests PRIVATE Catch2::Catch2 ${GUI_LIBS})


include(CTest)
include(Catch)
catch_discover_tests(tests)