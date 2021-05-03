cmake_minimum_required(VERSION 3.16)
project(3SpaceStudio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED)
find_package(ImGui-SFML REQUIRED)
find_package(wxwidgets REQUIRED)
find_package(glm REQUIRED)
find_package(span-lite REQUIRED)
find_package(Catch2 REQUIRED)
find_package(pegtl REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

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
file(GLOB VOL_SRC_FILES src/resources/*.cpp src/unvol/*.cpp)

file(GLOB STUDIO_SRC_FILES
        src/*.cpp
        src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp
        src/3space-studio/views/*.cpp src/3space-studio/*.cpp)

file(GLOB SHELL_SRC_FILES
        src/siege-shell/*.cpp)

file(GLOB LIB_SRC_FILES
        src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp)

list(REMOVE_ITEM DTS_SRC_FILES ${TEST_SRC_FILES})
list(REMOVE_ITEM JSON_SRC_FILES ${TEST_SRC_FILES})
list(REMOVE_ITEM OBJ_SRC_FILES ${TEST_SRC_FILES})
list(REMOVE_ITEM VOL_SRC_FILES ${TEST_SRC_FILES})
list(REMOVE_ITEM STUDIO_SRC_FILES ${TEST_SRC_FILES})
list(REMOVE_ITEM LIB_SRC_FILES ${TEST_SRC_FILES})

file(GLOB TESTABLE_SRC_FILES src/content/*.cpp
        src/content/**/*.cpp
        src/resources/*.cpp)

add_executable(dts-to-json ${DTS_SRC_FILES})
add_executable(dts-to-obj ${OBJ_SRC_FILES})
add_executable(json-to-dts ${JSON_SRC_FILES})
add_executable(unvol ${VOL_SRC_FILES})
add_executable(3space-studio ${STUDIO_SRC_FILES})
add_executable(siege-shell ${SHELL_SRC_FILES})
add_library(3space STATIC ${LIB_SRC_FILES})

#Include Directories and Linking Configuration
include_directories(packages/include)
include_directories(src)

foreach(target ${Boost_LIBRARIES_TARGETS})
    get_target_property(TARGET_INCLUDES ${target} INTERFACE_INCLUDE_DIRECTORIES)
    set(BOOST_INCLUDES ${BOOST_INCLUDES} ${TARGET_INCLUDES})
endforeach()

set(BASIC_INCLUDES ${BOOST_INCLUDES} ${Boost_INCLUDE_DIRS} ${nlohmann_json_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${nonstd_span-lite_INCLUDE_DIRS} ${taocpp_pegtl_INCLUDE_DIRS})
set(GUI_INCLUDES ${BASIC_INCLUDES} ${ImGui-SFML_INCLUDE_DIRS} ${wxwidgets_INCLUDE_DIRS})
set(GUI_LIBS ${ImGui-SFML_LIBRARIES} ${wxwidgets_LIBRARIES} ${glm_LIBRARIES})

target_include_directories(dts-to-json PRIVATE ${BASIC_INCLUDES})
target_include_directories(dts-to-obj PRIVATE ${BASIC_INCLUDES})
target_include_directories(json-to-dts PRIVATE ${BASIC_INCLUDES})

target_include_directories(unvol PRIVATE ${BASIC_INCLUDES})
target_include_directories(3space PRIVATE ${BASIC_INCLUDES})

target_include_directories(3space-studio PRIVATE ${GUI_INCLUDES})
target_include_directories(siege-shell PRIVATE ${GUI_INCLUDES})
target_link_libraries(3space-studio PRIVATE ${GUI_LIBS})
target_link_libraries(siege-shell PRIVATE ${GUI_LIBS})
add_custom_command(TARGET 3space-studio
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/src/3space-studio/logo.png $<TARGET_FILE_DIR:3space-studio>/logo.png
        )

add_custom_command(TARGET siege-shell
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/besieged-theme.png $<TARGET_FILE_DIR:3space-studio>/besieged-theme.png
        )

## Testing
add_executable(tests ${TESTABLE_SRC_FILES} ${TEST_SRC_FILES})
target_include_directories(tests PRIVATE ${GUI_INCLUDES} ${Catch2_INCLUDE_DIRS})
target_link_libraries(tests PRIVATE Catch2::Catch2 ${GUI_LIBS})

include(CTest)
include(Catch)
catch_discover_tests(tests)

## Final Target Settings
if(MSVC)
    target_sources(3space-studio PRIVATE src/3space-studio/3space-studio.rc)
    target_compile_options(dts-to-json PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(dts-to-obj PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(json-to-dts PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(unvol PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(siege-shell PRIVATE $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(3space PRIVATE $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(tests PRIVATE $<$<CONFIG:RELEASE>:/O2>)
else()
    target_compile_options(dts-to-json PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(dts-to-obj PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(json-to-dts PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(unvol PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(siege-shell PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(3space PRIVATE $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(tests PRIVATE $<$<CONFIG:RELEASE>:-O3>)
endif()

## Install/Packaging Section
file(GLOB_RECURSE HEADER_FILES src/*.hpp)
file(GLOB_RECURSE EXCLUDED_HEADER_FILES
        src/3space-studio/*.hpp)

list(REMOVE_ITEM HEADER_FILES ${EXCLUDED_HEADER_FILES})

add_custom_command(TARGET 3space-studio
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/include
        )

foreach(HEADER_FILE ${HEADER_FILES})
    file(RELATIVE_PATH HEADER_FILE_REL ${CMAKE_SOURCE_DIR}/src ${HEADER_FILE})
    add_custom_command(TARGET 3space-studio
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${HEADER_FILE} ${CMAKE_SOURCE_DIR}/include/${HEADER_FILE_REL}
            )
endforeach()

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(TARGETS 3space-studio 3space dts-to-json dts-to-obj json-to-dts
        CONFIGURATIONS Debug
        RUNTIME DESTINATION bin)

install(TARGETS 3space-studio 3space dts-to-json dts-to-obj json-to-dts
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin)

install(FILES src/3space-studio/logo.png DESTINATION bin)