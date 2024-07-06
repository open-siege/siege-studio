cmake_minimum_required(VERSION 3.20)
project(siege-platform)

add_library(siege-std STATIC src/std.cpp)
set_property(TARGET siege-std PROPERTY CXX_STANDARD 23)
target_include_directories(siege-std PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    add_library(siege-win-core STATIC src/win-core.cpp)
    set_property(TARGET siege-win-core PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-core PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_PC_APP)
    target_compile_definitions(siege-win-core PUBLIC COM_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_include_directories(siege-win-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-core PUBLIC OneCore mfplat mfreadwrite siege-std)

    add_library(siege-platform ALIAS siege-win-core)

    add_library(siege-win-gaming STATIC src/win-gaming.cpp)
    set_property(TARGET siege-win-gaming PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-gaming PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_GAMES)
    target_include_directories(siege-win-gaming PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-gaming PUBLIC siege-win-core)

    add_library(siege-win-desktop STATIC src/win-desktop.cpp src/theming-user.cpp src/theming-common.cpp)
    set_property(TARGET siege-win-desktop PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-desktop PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_include_directories(siege-win-desktop PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-desktop PUBLIC siege-win-gaming user32 comctl32 oleacc)

    install(TARGETS siege-std siege-win-core siege-win-gaming siege-win-desktop CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)

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

add_custom_target(Siege.Platform
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/Siege.Platform.dll
        COMMENT "Building Siege.Platform"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/$<$<CONFIG:Debug>:Debug>$<$<CONFIG:Release>:Release>/Siege.Platform.dll CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
