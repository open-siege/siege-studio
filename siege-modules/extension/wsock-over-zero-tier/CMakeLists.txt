cmake_minimum_required(VERSION 3.30)
project(wsock-over-zero-tier)

if (WIN32 AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/libzt)
    find_package(Catch2 REQUIRED)
    add_library(wsock32-on-zero-tier SHARED src/wsock32-on-zero-tier.cpp src/wsock32.def)
    target_include_directories(wsock32-on-zero-tier PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/libzt/include)
    set_target_properties(wsock32-on-zero-tier PROPERTIES CXX_STANDARD 23)
    target_link_libraries(wsock32-on-zero-tier PRIVATE siege-win-core)

    add_library(ws2_32-on-zero-tier SHARED src/wsock32-on-zero-tier.cpp src/ws2_32.def)
    target_compile_definitions(ws2_32-on-zero-tier PRIVATE USE_WINSOCK2=TRUE)
    target_include_directories(ws2_32-on-zero-tier PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/libzt/include)
    set_target_properties(ws2_32-on-zero-tier PROPERTIES CXX_STANDARD 23)
    target_link_libraries(ws2_32-on-zero-tier PRIVATE siege-win-core)

    add_library(wsock32-rpc-client SHARED src/wsock32-rpc-client.cpp src/wsock32.def)
    target_include_directories(wsock32-rpc-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set_target_properties(wsock32-rpc-client PROPERTIES CXX_STANDARD 23)
    target_link_libraries(wsock32-rpc-client PRIVATE siege-win-core)

    add_library(ws2_32-rpc-client SHARED src/wsock32-rpc-client.cpp src/ws2_32.def)
    target_include_directories(ws2_32-rpc-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set_target_properties(ws2_32-rpc-client PROPERTIES CXX_STANDARD 23)
    target_link_libraries(ws2_32-rpc-client PRIVATE siege-win-core)

    add_executable(wsock32-rpc-server WIN32 src/wsock32-rpc-server.cpp)
    target_include_directories(wsock32-rpc-server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    set_target_properties(wsock32-rpc-server PROPERTIES CXX_STANDARD 23)
    target_link_libraries(wsock32-rpc-server PRIVATE siege-win-core)

    add_executable(wsock32-rpc-tests src/wsock32-rpc.test.cpp)
    set_property(TARGET wsock32-rpc-tests PROPERTY CXX_STANDARD 23)
    target_include_directories(wsock32-rpc-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(wsock32-rpc-tests PRIVATE Catch2::Catch2WithMain siege-platform)

    include(CTest)
    include(Catch)
    catch_discover_tests(wsock32-rpc-tests)

    add_custom_command(TARGET wsock32-rpc-tests POST_BUILD 
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock32-rpc-client> $<TARGET_FILE_DIR:wsock32-rpc-tests>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock32-rpc-server> $<TARGET_FILE_DIR:wsock32-rpc-tests>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ws2_32-rpc-client> $<TARGET_FILE_DIR:wsock32-rpc-tests>)
endif()
