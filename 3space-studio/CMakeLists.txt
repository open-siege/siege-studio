project(3space-studio)
cmake_minimum_required(VERSION 3.17)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package(3space QUIET) # used for standalone builds. Requires lib3space to be packaged.
find_package(wxwidgets REQUIRED)
find_package(ImGui-SFML REQUIRED)

file(GLOB STUDIO_SRC_FILES
        src/*.cpp
        src/views/*.cpp)

add_executable(3space-studio ${STUDIO_SRC_FILES})
set_property(TARGET 3space-studio PROPERTY CXX_STANDARD 17)
target_include_directories(3space-studio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/packages/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(3space-studio 3space::3space wxwidgets::wxwidgets ImGui-SFML::ImGui-SFML)

if(MSVC)
    target_sources(3space-studio PRIVATE src/3space-studio.rc)
    target_compile_definitions(3space-studio PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

add_custom_command(TARGET 3space-studio
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/logo.png $<TARGET_FILE_DIR:3space-studio>/logo.png
        )

install(TARGETS 3space-studio
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)

install(FILES src/logo.png DESTINATION bin)
