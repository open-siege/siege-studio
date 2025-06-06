add_custom_target(${PROJECT_NAME}-dependencies ALL COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>)

add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}-dependencies)

set(INSTALLATION_PROJECTS siege-installation-dynamix 
                          siege-installation-other 
                          siege-installation-raven)
set(INST_TARGETS)

foreach(INST_PROJECT IN ITEMS ${INSTALLATION_PROJECTS})
    get_property(PROJECT_TARGETS DIRECTORY ${CMAKE_SOURCE_DIR}/siege-modules/installation/${INST_PROJECT} PROPERTY BUILDSYSTEM_TARGETS)
    list(APPEND INST_TARGETS ${PROJECT_TARGETS}) 
endforeach()

foreach(INST_TARGET IN ITEMS ${INST_TARGETS})
    get_target_property(TARGET_TYPE ${INST_TARGET} TYPE)
        if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY")
            continue()
        endif()
    add_dependencies(${PROJECT_NAME}-dependencies ${INST_TARGET})
    add_custom_command(TARGET ${PROJECT_NAME}-dependencies POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${INST_TARGET}> $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endforeach()