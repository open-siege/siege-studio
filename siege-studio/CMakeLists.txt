project(siege-studio)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.
find_package(wxwidgets REQUIRED)
find_package(ImGui-SFML REQUIRED)

file(GLOB STUDIO_SRC_FILES
        src/*.cpp
        src/content/**/*.cpp
        src/views/*.cpp)

add_subdirectory(tools)
add_executable(siege-studio ${STUDIO_SRC_FILES})
set_property(TARGET siege-studio PROPERTY CXX_STANDARD 17)
target_include_directories(siege-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/packages/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(siege-studio 3space::3space wxwidgets::wxwidgets ImGui-SFML::ImGui-SFML)

if(MSVC)
    target_sources(siege-studio PRIVATE src/siege-studio.rc)
    target_compile_definitions(siege-studio PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

if(UNIX AND NOT APPLE)
    find_package(TBB REQUIRED)
    target_link_libraries(siege-studio TBB::tbb)
endif()

add_custom_command(TARGET siege-studio
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/logo.png $<TARGET_FILE_DIR:siege-studio>/logo.png
        )

install(TARGETS siege-studio
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

install(FILES src/logo.png DESTINATION bin)
