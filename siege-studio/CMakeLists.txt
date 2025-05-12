cmake_minimum_required(VERSION 3.30)
project(siege-studio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
	add_library(siege-studio-core SHARED src/siege-studio-core.cpp src/siege-studio-core.def)
	target_compile_definitions(siege-studio-core PRIVATE UNICODE)
	target_link_libraries(siege-studio-core PUBLIC siege-win-desktop)

	add_library(siege-studio-core-static STATIC src/siege-studio-core.cpp)	
	target_compile_definitions(siege-studio-core-static PRIVATE UNICODE)
	target_link_libraries(siege-studio-core-static PUBLIC siege-win-desktop)

	add_executable(siege-studio WIN32 src/siege-studio.cpp src/siege-studio.manifest src/siege-studio.rc)
	target_compile_definitions(siege-studio PRIVATE UNICODE)
	target_link_libraries(siege-studio PRIVATE siege-studio-core-static)
else()
	add_executable(siege-studio src/linux.cpp)	
	target_link_libraries(siege-studio PRIVATE siege-platform)
endif()

set_property(TARGET siege-studio PROPERTY CXX_STANDARD 23)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
install(TARGETS siege-studio CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)