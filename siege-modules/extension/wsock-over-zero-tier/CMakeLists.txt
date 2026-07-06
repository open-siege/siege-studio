cmake_minimum_required(VERSION 3.30)
project(wsock-over-zero-tier)

if (WIN32 AND IS_DIRECTORY ${CMAKE_SOURCE_DIR}/libzt)
    find_package(Catch2 REQUIRED)

    add_library(wsock32-in-proc-client SHARED src/wsock32-common-shared.cpp src/wsock32-client-shared.cpp src/wsock32-in-proc-client.cpp src/wsock32.def)
    target_include_directories(wsock32-in-proc-client PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/libzt/include)
    set_target_properties(wsock32-in-proc-client PROPERTIES         
         CXX_STANDARD 23
         CXX_STANDARD_REQUIRED ON
         CXX_SCAN_FOR_MODULES ON
     )
    target_link_libraries(wsock32-in-proc-client PRIVATE siege-win-core)

    if(TARGET zt-static)
        add_library(wsock-backend-zero-tier SHARED src/wsock32-common-shared.cpp src/wsock-backend-zero-tier.cpp src/backend.def)
        target_compile_definitions(wsock-backend-zero-tier PRIVATE USE_WINSOCK2=TRUE)
        target_include_directories(wsock-backend-zero-tier PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/libzt/include)
        set_target_properties(wsock-backend-zero-tier PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_SCAN_FOR_MODULES ON
        )
        target_link_libraries(wsock-backend-zero-tier PRIVATE siege-win-core zt-static)
    endif()


    add_library(wsock32-rpc-client SHARED src/wsock32-common-shared.cpp src/wsock32-client-shared.cpp src/wsock32-rpc.cpp src/wsock32-rpc-client.cpp src/dll-main.cpp src/wsock32.def)
    set_target_properties(wsock32-rpc-client PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
    )
    target_link_libraries(wsock32-rpc-client PRIVATE siege-win-core siege-extension ws2_32)

    add_library(ws2_32-rpc-client SHARED src/wsock32-common-shared.cpp src/wsock32-client-shared.cpp src/wsock32-rpc.cpp src/wsock32-rpc-client.cpp src/dll-main.cpp src/ws2_32.def)
    set_target_properties(ws2_32-rpc-client PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
    )
    target_compile_definitions(ws2_32-rpc-client PRIVATE USE_WINSOCK2=TRUE)
    target_link_libraries(ws2_32-rpc-client PRIVATE siege-win-core siege-extension ws2_32)

    add_executable(wsock32-rpc-server WIN32 src/wsock32-rpc.cpp src/wsock32-rpc-server.cpp)
    set_target_properties(wsock32-rpc-server PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
    )
    target_link_libraries(wsock32-rpc-server PRIVATE siege-win-core ws2_32)

    add_executable(wsock32-rpc-tests src/wsock32-rpc.test.cpp)
    set_target_properties(wsock32-rpc-tests PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
    )
    target_include_directories(wsock32-rpc-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(wsock32-rpc-tests PRIVATE Catch2::Catch2WithMain siege-platform ws2_32)

    add_executable(wsock32-e2e-tests src/wsock32-common-shared.cpp src/wsock32-e2e.test.cpp)
        set_target_properties(wsock32-e2e-tests PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_SCAN_FOR_MODULES ON
    )
    target_include_directories(wsock32-e2e-tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(wsock32-e2e-tests PRIVATE siege-platform)

    add_dependencies(wsock32-e2e-tests wsock32-in-proc-client wsock-backend-zero-tier)
    add_custom_command(TARGET wsock32-e2e-tests POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock32-in-proc-client> $<TARGET_FILE_DIR:wsock32-e2e-tests>
       COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock-backend-zero-tier> $<TARGET_FILE_DIR:wsock-backend-zero-tier>)

    include(CTest)
    include(Catch)
    catch_discover_tests(wsock32-rpc-tests)

    add_custom_command(TARGET wsock32-rpc-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock32-rpc-client> $<TARGET_FILE_DIR:wsock32-rpc-tests>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wsock32-rpc-server> $<TARGET_FILE_DIR:wsock32-rpc-tests>
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ws2_32-rpc-client> $<TARGET_FILE_DIR:wsock32-rpc-tests>)
endif()
