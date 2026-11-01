cmake_minimum_required(VERSION 3.30)
project(siege-platform)

add_library(siege-platform STATIC
    src/platform_shared.cpp
    src/endian.cpp
    src/tagged_data.cpp
    src/wave.cpp
    src/stream.cpp
    src/bitmap.cpp
    src/palette.cpp
    src/resource.cpp
    src/http.cpp)

if (WIN32)
    target_sources(siege-platform PRIVATE
        src/vkey.win32.cpp
        src/http.win32.cpp
        src/platform_image.win32.cpp
        src/win32/win-desktop.cpp
        src/win32/com.cpp
        src/win32/process.cpp
        src/win32/utils.cpp
        src/win32/module.cpp
        src/win32/file.cpp
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
        src/extension_module.cpp
        src/installation_module.cpp
        src/presentation_module.cpp
        src/storage_module.cpp)

    # FSL == filesystem literal. Allows Windows to have UTF16 literals by default.
    target_compile_definitions(siege-platform PUBLIC "FSL=L\"\"")
    target_compile_definitions(siege-platform PUBLIC COM_NO_WINDOWS_H DWRITE_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_compile_definitions(siege-platform PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_link_libraries(siege-platform PUBLIC OneCore user32 winhttp windowscodecs)

    add_library(siege-win-core ALIAS siege-platform)
    add_library(siege-win-desktop ALIAS siege-platform)
else()
    target_compile_definitions(siege-platform PUBLIC FSL=)
    target_sources(siege-platform PRIVATE src/vkey.cpp)
endif()

set_property(TARGET siege-platform PROPERTY CXX_STANDARD 23)
set_property(TARGET siege-platform PROPERTY CXX_SCAN_FOR_MODULES ON)
target_include_directories(siege-platform PUBLIC ${PROJECT_SOURCE_DIR}/include)


install(TARGETS siege-platform CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
