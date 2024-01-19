cmake_minimum_required(VERSION 3.20)

set(CMAKE_SYSTEM_NAME WindowsStore)
set(CMAKE_SYSTEM_VERSION 10.0)
project(siege-launcher-uwp)

set_directory_properties(PROPERTIES VS_STARTUP_PROJECT ${PROJECT_NAME})

file(GLOB ASSET_FILES ${PROJECT_SOURCE_DIR}/Assets/*.*)
set_source_files_properties(Package.appxmanifest ${ASSET_FILES} PROPERTIES VS_DEPLOYMENT_CONTENT 1)
set_source_files_properties(${ASSET_FILES} PROPERTIES VS_DEPLOYMENT_LOCATION "Assets")

add_executable(siege-launcher-uwp WIN32 App.cpp Package.appxmanifest ${ASSET_FILES})

set_target_properties(siege-launcher-uwp
  PROPERTIES 
  CXX_STANDARD 23
  VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION 10.0.18362.0)

target_compile_definitions(siege-launcher-uwp PRIVATE UNICODE)
target_include_directories(siege-launcher-uwp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS siege-launcher-uwp CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
