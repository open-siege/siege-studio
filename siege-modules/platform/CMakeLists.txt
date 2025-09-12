cmake_minimum_required(VERSION 3.30)
project(siege-platform)

add_library(siege-std STATIC src/std.cpp src/bitmap.cpp src/palette.cpp src/image.cpp src/module.cpp src/resource.cpp)
set_property(TARGET siege-std PROPERTY CXX_STANDARD 23)
target_include_directories(siege-std PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    add_library(siege-win-core STATIC src/win-core.cpp)
    set_property(TARGET siege-win-core PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-core PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_PC_APP)
    target_compile_definitions(siege-win-core PUBLIC COM_NO_WINDOWS_H DWRITE_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_include_directories(siege-win-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-core PUBLIC OneCore siege-std)

    add_library(siege-platform ALIAS siege-win-core)

    add_library(siege-win-desktop STATIC src/win-desktop.cpp 
        src/theming-user.cpp 
        src/theming-common.cpp  
        src/theming-shared.cpp 
        src/theming-masking.cpp
        src/theming-font.cpp
        src/layout-common.cpp
        src/registry.cpp
        src/platform_image.win32.cpp
        src/http.win32.cpp)
    set_property(TARGET siege-win-desktop PROPERTY CXX_STANDARD 23)
    target_compile_definitions(siege-win-desktop PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_include_directories(siege-win-desktop PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-desktop PUBLIC siege-win-core user32)

    install(TARGETS siege-std siege-win-core siege-win-desktop CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
else()
    # TODO make GTK or SDL the other platform
    add_library(siege-platform ALIAS siege-std)
endif()