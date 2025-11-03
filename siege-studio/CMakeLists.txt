cmake_minimum_required(VERSION 3.30)
project(siege-studio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

string(TIMESTAMP SIEGE_MAJOR_VERSION "%y")
string(TIMESTAMP SIEGE_MINOR_VERSION "%V")

if (NOT DEFINED SIEGE_CHANNEL_TYPE)
	set(SIEGE_CHANNEL_TYPE "\"development\"")
endif()

if (WIN32)
	set(STUDIO_FILES src/siege-studio-core.cpp src/views/about_view.cpp src/views/default_view.cpp src/views/preferences_view.cpp src/views/siege_main_window.cpp src/views/stack_layout.cpp)
	add_library(siege-studio-core SHARED ${STUDIO_FILES} src/siege-studio-core.def)
	target_compile_definitions(siege-studio-core PRIVATE UNICODE SIEGE_MAJOR_VERSION=${SIEGE_MAJOR_VERSION} SIEGE_MINOR_VERSION=${SIEGE_MINOR_VERSION})
	target_link_libraries(siege-studio-core PUBLIC siege-win-desktop Imagehlp)

	add_library(siege-studio-core-static STATIC ${STUDIO_FILES})	
	target_compile_definitions(siege-studio-core-static PRIVATE UNICODE SIEGE_MAJOR_VERSION=${SIEGE_MAJOR_VERSION} SIEGE_MINOR_VERSION=${SIEGE_MINOR_VERSION})
	target_link_libraries(siege-studio-core-static PUBLIC siege-win-desktop Imagehlp)

	add_executable(siege-studio WIN32 src/siege-studio.cpp src/app-update.cpp src/siege-studio.manifest src/siege-studio.rc)
	target_compile_definitions(siege-studio PRIVATE UNICODE
	SIEGE_CHANNEL_TYPE=${SIEGE_CHANNEL_TYPE}
	SIEGE_MAJOR_VERSION=${SIEGE_MAJOR_VERSION} SIEGE_MINOR_VERSION=${SIEGE_MINOR_VERSION})
	target_link_libraries(siege-studio PRIVATE siege-studio-core-static Imagehlp)
else()
	add_executable(siege-studio src/linux.cpp)	
	target_link_libraries(siege-studio PRIVATE siege-platform)
endif()

set_property(TARGET siege-studio PROPERTY CXX_STANDARD 23)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
include(copy_deps.cmake)

install(TARGETS siege-studio CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)