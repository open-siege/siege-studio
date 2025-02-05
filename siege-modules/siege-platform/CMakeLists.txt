cmake_minimum_required(VERSION 3.28)
project(siege-platform)

add_library(siege-std STATIC src/std.cpp src/bitmap.cpp src/palette.cpp src/image.cpp)
set_property(TARGET siege-std PROPERTY CXX_STANDARD 23)
target_include_directories(siege-std PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    add_library(siege-win-core STATIC src/win-core.cpp)
    set_property(TARGET siege-win-core PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-core PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_PC_APP)
    target_compile_definitions(siege-win-core PUBLIC COM_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_include_directories(siege-win-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-core PUBLIC OneCore mfplat mfreadwrite Windowscodecs D2d1 Dwrite siege-std)

    add_library(siege-platform ALIAS siege-win-core)

    add_library(siege-win-desktop STATIC src/win-desktop.cpp 
        src/theming-user.cpp 
        src/theming-common.cpp 
        src/theming-masking.cpp
        src/theming-font.cpp
        src/platform_image.win32.cpp)
    set_property(TARGET siege-win-desktop PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-desktop PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_include_directories(siege-win-desktop PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-desktop PUBLIC siege-win-core user32 comctl32 dwmapi)

    install(TARGETS siege-std siege-win-core siege-win-desktop CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)

    # Tests
    find_package(Catch2 REQUIRED)
    add_executable(com-tests src/com-base-tests.cpp)
    set_property(TARGET com-tests PROPERTY CXX_STANDARD 23)
    target_include_directories(com-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(com-tests PRIVATE Catch2::Catch2WithMain siege-win-core)

    include(CTest)
    include(Catch)
    catch_discover_tests(com-tests)

    add_custom_command(
         TARGET com-tests
         COMMENT "Running com-tests"
         POST_BUILD 
         WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
         COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --output-on-failures
    )

else()
    # TODO make GTK or SDL the other platform
    add_library(siege-platform ALIAS siege-std)
endif()