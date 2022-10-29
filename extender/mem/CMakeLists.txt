project(siege-mem)
cmake_minimum_required(VERSION 3.17)

set(PKG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/impl/package/lib/mem.dll)

add_custom_target(siege-mem ALL conan package .
        BYPRODUCTS ${PKG_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/impl)

add_custom_command(TARGET siege-mem
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${PKG_FILES} ${CMAKE_CURRENT_BINARY_DIR}
        )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/mem.dll
        CONFIGURATIONS Debug Release
        DESTINATION lib)
