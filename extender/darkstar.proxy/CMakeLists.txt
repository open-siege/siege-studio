project(darkstar-proxy)
cmake_minimum_required(VERSION 3.22)

set(PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/lib/darkstar.proxy.dll ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/lib/darkstar.proxy.lib)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/include)

add_custom_target(darkstar-proxy-build ALL conan package .
        BYPRODUCTS ${PKG_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/impl)

add_custom_command(TARGET darkstar-proxy-build
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PKG_FILES} ${CMAKE_CURRENT_BINARY_DIR}
        )

add_library(darkstar-proxy SHARED IMPORTED GLOBAL)
set_property(TARGET darkstar-proxy PROPERTY
        IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/darkstar.proxy.dll)
set_property(TARGET darkstar-proxy PROPERTY
        IMPORTED_IMPLIB ${CMAKE_CURRENT_BINARY_DIR}/darkstar.proxy.lib)
set_property(TARGET darkstar-proxy PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/include)

add_dependencies(darkstar-proxy darkstar-proxy-build)

install(DIRECTORY impl/package/include
        DESTINATION .
        COMPONENT devel
        FILES_MATCHING PATTERN "*.hpp")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/darkstar.proxy.dll ${CMAKE_CURRENT_BINARY_DIR}/darkstar.proxy.lib
        CONFIGURATIONS Debug Release
        DESTINATION lib)

