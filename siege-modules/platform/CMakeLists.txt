cmake_minimum_required(VERSION 3.30)
project(siege-platform)

add_library(siege-std STATIC
    src/std.cpp
    src/platform_shared.cpp
    src/endian.cpp
    src/tagged_data.cpp
    src/wave.cpp
    src/stream.cpp
    src/bitmap.cpp
    src/palette.cpp
    src/win32/utils.cpp
    src/win32/module.cpp
    src/win32/file.cpp
    src/resource.cpp)
set_property(TARGET siege-std PROPERTY CXX_STANDARD 23)
set_property(TARGET siege-std PROPERTY CXX_SCAN_FOR_MODULES ON)
target_include_directories(siege-std PUBLIC ${PROJECT_SOURCE_DIR}/include)
# Native path string literal prefix: L"" on Windows, empty elsewhere (was shared.hpp FSL).
if (WIN32)
    target_compile_definitions(siege-std PUBLIC "FSL=L\"\"")
else()
    target_compile_definitions(siege-std PUBLIC FSL=)
endif()

if (WIN32)
    add_library(siege-win-core STATIC
        src/win-core.cpp
        src/win32/com.cpp
        src/win32/process.cpp
        src/extension_module.cpp
        src/installation_module.cpp)
    set_property(TARGET siege-win-core PROPERTY CXX_STANDARD 23)
    set_property(TARGET siege-win-core PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(siege-win-core PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_PC_APP)
    target_compile_definitions(siege-win-core PUBLIC COM_NO_WINDOWS_H DWRITE_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_include_directories(siege-win-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-core PUBLIC OneCore siege-std)

    add_library(siege-platform ALIAS siege-win-core)

    add_library(siege-win-desktop STATIC
        src/win-desktop.cpp
        src/win32/window.cpp
        src/win32/dialog.cpp
        src/win32/user_controls.cpp
        src/win32/common_controls.cpp
        src/win32/shell.cpp
        src/win32/theming.cpp
        src/win32/theming-user.cpp
        src/win32/theming-common.cpp
        src/win32/theming-shared.cpp
        src/win32/theming-masking.cpp
        src/win32/theming-font.cpp
        src/win32/layout.cpp
        src/win32/drawing.cpp
        src/win32/shell_dialogs.cpp
        src/win32/wic.cpp
        src/win32/threading.cpp
        src/win32/registry.cpp
        src/win32/capabilities.cpp
        src/platform_image.win32.cpp
        src/http.cpp
        src/http.win32.cpp
        src/presentation_module.cpp
        src/storage_module.cpp)
    set_property(TARGET siege-win-desktop PROPERTY CXX_STANDARD 23)
    set_property(TARGET siege-win-desktop PROPERTY CXX_SCAN_FOR_MODULES ON)
    target_compile_definitions(siege-win-desktop PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_include_directories(siege-win-desktop PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-desktop PUBLIC siege-win-core user32 winhttp windowscodecs)

    install(TARGETS siege-std siege-win-core siege-win-desktop CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
else()
    # TODO make GTK or SDL the other platform
    add_library(siege-platform ALIAS siege-std)
endif()
