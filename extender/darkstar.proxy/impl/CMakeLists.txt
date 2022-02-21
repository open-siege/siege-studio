cmake_minimum_required(VERSION 3.16)
project(darkstar-proxy-impl)

add_library(darkstar.proxy SHARED src/Proxy.cpp src/darkstar.proxy.def)
target_include_directories(darkstar.proxy PUBLIC include)
set_property(TARGET darkstar.proxy PROPERTY CXX_STANDARD 17)

target_link_options(darkstar.proxy PRIVATE -static)
target_link_libraries(darkstar.proxy -static-libgcc -static-libstdc++)

add_custom_command(TARGET darkstar.proxy
        POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/lib /def:${CMAKE_CURRENT_SOURCE_DIR}/src/darkstar.proxy.def /out:$<TARGET_FILE_DIR:darkstar.proxy>/darkstar.proxy.lib /machine:x86
        )

add_custom_command(TARGET darkstar.proxy
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE_DIR:darkstar.proxy>/libdarkstar.proxy.dll $<TARGET_FILE_DIR:darkstar.proxy>/darkstar.proxy.dll
        )

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(FILES $<TARGET_FILE_DIR:darkstar.proxy>/darkstar.proxy.dll $<TARGET_FILE_DIR:darkstar.proxy>/darkstar.proxy.lib
        CONFIGURATIONS Debug Release
        DESTINATION lib)
