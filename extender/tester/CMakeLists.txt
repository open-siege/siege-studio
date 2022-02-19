cmake_minimum_required(VERSION 3.16)
project(darkstar-extender-tester)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
find_package(wxwidgets REQUIRED)

add_executable(extender-tester src/main.cpp)
set_property(TARGET extender-tester PROPERTY CXX_STANDARD 17)
target_link_libraries(extender-tester PRIVATE wxwidgets::wxwidgets)

install(TARGETS extender-tester CONFIGURATIONS Debug Release RUNTIME DESTINATION bin)
