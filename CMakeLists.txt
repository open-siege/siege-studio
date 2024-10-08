cmake_minimum_required(VERSION 3.20)
project(siege-frontend)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_subdirectory(siege-modules)
add_subdirectory(siege-tools)
add_subdirectory(siege-studio)

add_custom_target(siege-studio-dependencies ALL COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:siege-studio>)
add_custom_command(TARGET siege-studio-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:unvol> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dts-to-json> $<TARGET_FILE_DIR:siege-studio>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:dts-to-obj> $<TARGET_FILE_DIR:siege-studio>)

set(CONTENT_TARGETS siege-content-2d 
                    siege-content-3d 
                    siege-content-resource 
                    siege-content-audio 
                    siege-content-configuration
                    siege-content-executable)

foreach(CONTENT_TARGET IN ITEMS ${CONTENT_TARGETS})
            add_dependencies(siege-studio-dependencies ${CONTENT_TARGET})
            add_custom_command(TARGET siege-studio-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${CONTENT_TARGET}> $<TARGET_FILE_DIR:siege-studio>)
endforeach()

if (WIN32)
        get_property(ID_TECH_TARGETS
                DIRECTORY siege-modules/siege-extension-id-tech PROPERTY BUILDSYSTEM_TARGETS)

        get_property(DARKSTAR_TARGETS
                DIRECTORY siege-modules/siege-extension-darkstar PROPERTY BUILDSYSTEM_TARGETS)


        set(EXT_TARGETS)
        list(APPEND EXT_TARGETS ${ID_TECH_TARGETS} ${DARKSTAR_TARGETS} 
        siege-extension-generic
        siege-extension-input-filter-raw-input)

        foreach(EXT_TARGET IN ITEMS ${EXT_TARGETS})
            add_dependencies(siege-studio-dependencies ${EXT_TARGET})
            add_custom_command(TARGET siege-studio-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXT_TARGET}> $<TARGET_FILE_DIR:siege-studio>)
        endforeach()
endif()

add_dependencies(siege-studio-dependencies 
        siege-content-2d 
        siege-content-3d 
        siege-content-resource 
        siege-content-audio 
        dts-to-json
        dts-to-obj
        game-unpack
        unvol)
add_dependencies(siege-studio siege-studio-dependencies)

if (IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/siege-launcher)
    add_subdirectory(siege-launcher)
    add_custom_target(siege-launcher-dependencies ALL COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:siege-launcher>)
    add_dependencies(siege-launcher-dependencies siege-studio-dependencies siege-configuration)
    add_dependencies(siege-launcher siege-launcher-dependencies)

    add_custom_command(TARGET siege-launcher-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-content-2d> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-content-3d> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-content-resource> $<TARGET_FILE_DIR:siege-launcher>
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-content-audio> $<TARGET_FILE_DIR:siege-launcher>)

    if (WIN32)
        foreach(EXT_TARGET IN ITEMS ${EXT_TARGETS})
            add_dependencies(siege-launcher-dependencies ${EXT_TARGET})
            add_custom_command(TARGET siege-launcher-dependencies POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${EXT_TARGET}> $<TARGET_FILE_DIR:siege-launcher>)
        endforeach()
    endif()
endif()