cmake_minimum_required(VERSION 3.16)
project(detours)

add_library(detours STATIC src/detours.cpp
        src/modules.cpp
        src/disasm.cpp
        src/image.cpp
        src/creatwth.cpp
        src/disolx86.cpp
        src/disolx64.cpp
        src/disolia64.cpp
        src/disolarm.cpp
        src/disolarm64.cpp)

target_link_libraries(detours)
target_include_directories(detours PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB HEADER_FILES src/*.h)

add_custom_command(TARGET detours
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

add_custom_command(TARGET detours
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${HEADER_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.h")

install(TARGETS detours
        CONFIGURATIONS Debug Release
        RUNTIME DESTINATION bin)
