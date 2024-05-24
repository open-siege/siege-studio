cmake_minimum_required(VERSION 3.20)
project(Siege.Platform VERSION 1.0.0.0 LANGUAGES CSharp)

add_custom_target(${PROJECT_NAME}
        COMMAND dotnet build "${CMAKE_CURRENT_SOURCE_DIR}" --output "${CMAKE_CURRENT_BINARY_DIR}"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll
        COMMENT "Building Siege.Platform"
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.dll CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)