project(siege-proxy)
cmake_minimum_required(VERSION 3.22)

set(PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/lib/siege.proxy.dll ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/lib/siege.proxy.lib)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/include)

add_custom_target(siege-proxy-build ALL conan package .
        BYPRODUCTS ${PKG_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/impl)

add_custom_command(TARGET siege-proxy-build
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PKG_FILES} ${CMAKE_CURRENT_BINARY_DIR}
        )

add_library(siege-proxy SHARED IMPORTED GLOBAL)
set_property(TARGET siege-proxy PROPERTY
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/siege.proxy.dll)
set_property(TARGET siege-proxy PROPERTY
        IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/siege.proxy.lib)
set_property(TARGET siege-proxy PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/include)

add_dependencies(siege-proxy siege-proxy-build)

install(DIRECTORY impl/package/include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/siege.proxy.dll ${CMAKE_CURRENT_BINARY_DIR}/siege.proxy.lib
        CONFIGURATIONS Debug Release
        DESTINATION lib)

