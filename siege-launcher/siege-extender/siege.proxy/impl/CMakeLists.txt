cmake_minimum_required(VERSION 3.16)
project(siege-proxy-impl)

add_library(siege.proxy SHARED src/Proxy.cpp src/siege.proxy.def)
target_include_directories(siege.proxy PUBLIC include)
set_property(TARGET siege.proxy PROPERTY CXX_STANDARD 17)

target_link_options(siege.proxy PRIVATE -static)
target_link_libraries(siege.proxy -static-libgcc -static-libstdc++)

add_custom_command(TARGET siege.proxy
        POST_BUILD
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/lib /def:${CMAKE_CURRENT_SOURCE_DIR}/src/siege.proxy.def /out:$<TARGET_FILE_DIR:siege.proxy>/siege.proxy.lib /machine:x86
        )

add_custom_command(TARGET siege.proxy
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rename $<TARGET_FILE_DIR:siege.proxy>/libsiege.proxy.dll $<TARGET_FILE_DIR:siege.proxy>/siege.proxy.dll
        )

install(DIRECTORY include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(FILES $<TARGET_FILE_DIR:siege.proxy>/siege.proxy.dll $<TARGET_FILE_DIR:siege.proxy>/siege.proxy.lib
        CONFIGURATIONS Debug Release
        DESTINATION lib)
