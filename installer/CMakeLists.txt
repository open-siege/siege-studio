cmake_minimum_required(VERSION 3.16)
project(StarsiegeLauncher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

find_package(wxwidgets REQUIRED)
find_package(Catch2 REQUIRED)

file(GLOB_RECURSE TEST_SRC_FILES src/*.test.cpp)

file(GLOB INSTALLER_SRC_FILES
        src/installer/*.cpp)

list(REMOVE_ITEM INSTALLER_SRC_FILES ${TEST_SRC_FILES})

add_executable(starsiege-installer ${INSTALLER_SRC_FILES})

target_include_directories(starsiege-installer PRIVATE src/installer ${GUI_INCLUDES} ${wxwidgets_INCLUDE_DIRS})
target_link_libraries(starsiege-installer PRIVATE ${wxwidgets_LIBRARIES})

## Testing
add_executable(tests ${TEST_SRC_FILES})
target_include_directories(tests PRIVATE ${Catch2_INCLUDES} ${wxwidgets_INCLUDE_DIRS})
target_link_libraries(tests PRIVATE Catch2::Catch2 ${wxwidgets_LIBRARIES})

include(CTest)
include(Catch)
catch_discover_tests(tests)

## Final Target Settings
if(MSVC)
    target_compile_options(starsiege-installer PRIVATE $<$<CONFIG:RELEASE>:/O2>)
else()
    target_compile_options(starsiege-installer PRIVATE $<$<CONFIG:RELEASE>:-O3>)
endif()

## Install/Packaging Section
install(TARGETS starsiege-installer
        CONFIGURATIONS Debug
        RUNTIME DESTINATION bin)

install(TARGETS starsiege-installer
        CONFIGURATIONS Release
        RUNTIME DESTINATION bin)