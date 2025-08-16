project(unvol)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
    add_executable(${PROJECT_NAME} src/unvol.cpp src/unvol.win32.cpp src/unvol.rc)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop)
else()
    add_executable(${PROJECT_NAME} src/unvol.cpp src/unvol.other.cpp)
endif()


set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} PRIVATE siege-content-all)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(unvol TBB::tbb)
endif()

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
