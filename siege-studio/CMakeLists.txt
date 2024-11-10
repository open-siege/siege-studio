cmake_minimum_required(VERSION 3.28)
project(siege-studio)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
	add_executable(siege-studio WIN32 src/siege-studio.cpp src/siege-studio.manifest src/siege-studio.rc)
	target_compile_definitions(siege-studio PRIVATE UNICODE)
	target_link_libraries(siege-studio PRIVATE siege-win-desktop)
else()
	add_executable(siege-studio src/linux.cpp)	
	target_link_libraries(siege-studio PRIVATE siege-platform)
endif()

set_property(TARGET siege-studio PROPERTY CXX_STANDARD 23)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
	
install(TARGETS siege-studio CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)