project(unvol)
cmake_minimum_required(VERSION 3.30)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
    add_executable(${PROJECT_NAME} src/unvol.cpp src/unvol.win32.cpp src/resource_maker.cpp src/unvol.rc)
    target_link_libraries(${PROJECT_NAME} PRIVATE siege-win-desktop)
else()
    add_executable(${PROJECT_NAME} src/unvol.cpp src/unvol.other.cpp src/resource_maker.cpp)
endif()


set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_link_libraries(${PROJECT_NAME} PRIVATE 
                                    siege-content-3space-resource
                                    siege-content-id-tech-resource
                                    siege-content-sword-resource
                                    siege-content-mw2-resource
                                    siege-content-mw4-resource
                                    siege-content-colony-wars-resource
                                    siege-content-cyclone-resource)

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(unvol TBB::tbb)
endif()

install(TARGETS ${PROJECT_NAME}
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
