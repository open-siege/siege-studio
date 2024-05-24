cmake_minimum_required(VERSION 3.20)
project(nuvol VERSION 1.0.0.0 LANGUAGES CSharp)

add_custom_target(${PROJECT_NAME}
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe
        DEPENDS Siege.Resource.Content
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:siege-resource-content> ${CMAKE_CURRENT_BINARY_DIR})


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.exe CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)