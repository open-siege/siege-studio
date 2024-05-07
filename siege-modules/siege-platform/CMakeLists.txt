cmake_minimum_required(VERSION 3.20)
project(siege-platform)

add_library(siege-std STATIC src/std.cpp)
set_property(TARGET siege-std PROPERTY CXX_STANDARD 23)
set_property(TARGET siege-std PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(siege-std PUBLIC ${PROJECT_SOURCE_DIR}/include)

if (WIN32)
    add_library(siege-win-core STATIC src/win-core.cpp)
    set_property(TARGET siege-win-core PROPERTY CXX_STANDARD 23)
    set_property(TARGET siege-win-core PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(siege-win-core PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_PC_APP)
    target_compile_definitions(siege-win-core PUBLIC COM_NO_WINDOWS_H NOMINMAX STRICT UNICODE)
    target_include_directories(siege-win-core PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-core PUBLIC OneCore siege-std)

    add_library(siege-platform ALIAS siege-win-core)

    add_library(siege-win-gaming STATIC src/win-games.cpp)
    set_property(TARGET siege-win-gaming PROPERTY CXX_STANDARD 23)
    set_property(TARGET siege-win-gaming PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(siege-win-gaming PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_GAMES)
    target_include_directories(siege-win-gaming PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-gaming PUBLIC siege-win-core)

    add_library(siege-win-desktop STATIC src/win-desktop.cpp)
    set_property(TARGET siege-win-desktop PROPERTY CXX_STANDARD 23)
    set_property(TARGET siege-win-desktop PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(siege-win-desktop PRIVATE WINAPI_FAMILY=WINAPI_FAMILY_DESKTOP_APP)
    target_include_directories(siege-win-desktop PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_link_libraries(siege-win-desktop PUBLIC siege-win-gaming user32 comctl32 oleacc)

    add_library(siege-win-mfc SHARED src/win-mfc.cpp src/win-mfc.rc)
    set_property(TARGET siege-win-mfc PROPERTY CXX_STANDARD 17)
    set_property(TARGET siege-win-mfc PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(siege-win-mfc PRIVATE UNICODE VC_EXTRALEAN _ATL_CSTRING_EXPLICIT_CONSTRUCTORS _AFX_NO_DAO_SUPPORT)
    target_include_directories(siege-win-mfc PUBLIC ${PROJECT_SOURCE_DIR}/include)

    install(TARGETS siege-std siege-win-core siege-win-gaming siege-win-desktop siege-win-mfc CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
else()
    # TODO make GTK or SDL the other platform
    add_library(siege-platform ALIAS siege-std)
endif()

