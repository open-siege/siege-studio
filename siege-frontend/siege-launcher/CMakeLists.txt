cmake_minimum_required(VERSION 3.20)
project(siege-launcher)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(WIN32)
    find_package(vigemclient REQUIRED)
    add_executable(siege-launcher src/main.cpp)

    target_link_libraries(siege-launcher PRIVATE vigemclient::vigemclient)
else()
    add_executable(siege-launcher src/main.cpp)
endif()

set_property(TARGET siege-launcher PROPERTY CXX_STANDARD 23)
target_include_directories(siege-launcher PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(siege-launcher PRIVATE siege-input siege-interface)

install(TARGETS siege-launcher CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
