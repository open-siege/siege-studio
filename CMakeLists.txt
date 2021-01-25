cmake_minimum_required(VERSION 3.16)
project(DarkstarDtsConverter)
include(conanbuildinfo.cmake)
conan_basic_setup()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

#file(GLOB MDL_SRC_FILES src/mdl-to-json/*.cpp)
file(GLOB DTS_SRC_FILES src/dts-to-json/*.cpp)
file(GLOB OBJ_SRC_FILES src/dts_renderable_shape.cpp src/dts-to-obj/*.cpp)
file(GLOB JSON_SRC_FILES src/json-to-dts/*.cpp)
file(GLOB VOL_SRC_FILES src/resource/*.cpp src/unvol/*.cpp)
file(GLOB MIS_SRC_FILES src/mis-to-json/*.cpp)
file(GLOB DTS_VIEWER_SRC_FILES
        src/*.cpp
        src/content/*.cpp
        src/content/**/*.cpp
        src/resource/*.cpp
        src/3space-studio/views/*.cpp src/3space-studio/*.cpp)

#add_executable(mdl-to-json ${MDL_SRC_FILES})
#add_executable(dts-to-json ${DTS_SRC_FILES})
add_executable(dts-to-obj ${OBJ_SRC_FILES})
#add_executable(json-to-dts ${JSON_SRC_FILES})
#add_executable(unvol ${VOL_SRC_FILES})
#add_executable(mis-to-json ${MIS_SRC_FILES})
add_executable(3space-studio ${DTS_VIEWER_SRC_FILES})

include_directories(${CONAN_INCLUDE_DIRS})
include_directories(packages/include)
include_directories(src)

#conan_target_link_libraries(mdl-to-json)
#conan_target_link_libraries(dts-to-json)
conan_target_link_libraries(dts-to-obj)
#conan_target_link_libraries(json-to-dts)
#conan_target_link_libraries(unvol)
#conan_target_link_libraries(mis-to-json)
target_link_libraries(3space-studio PRIVATE ${CONAN_LIBS})

if(MSVC)
#    target_compile_options(mdl-to-json PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
#    target_compile_options(dts-to-json PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(dts-to-obj PRIVATE /W3 /WX $<$<CONFIG:RELEASE>:/O2>)
#    target_compile_options(json-to-dts PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
#    target_compile_options(unvol PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
#    target_compile_options(mis-to-json PRIVATE /W4 /WX $<$<CONFIG:RELEASE>:/O2>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:/O2>)
else()
#    target_compile_options(mdl-to-json PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
#    target_compile_options(dts-to-json PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(dts-to-obj PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
#    target_compile_options(json-to-dts PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
#    target_compile_options(unvol PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
#    target_compile_options(mis-to-json PRIVATE -Wall -Wextra -Werror -pedantic $<$<CONFIG:RELEASE>:-O3>)
    target_compile_options(3space-studio PRIVATE $<$<CONFIG:RELEASE>:-O3>)
endif()