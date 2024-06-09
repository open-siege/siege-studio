cmake_minimum_required(VERSION 3.20)
project(siege-extension-id-tech)

if (WIN32)
    add_library(siege-extension-quake SHARED src/sof.cpp src/id-tech-2.def)
    set_target_properties(siege-extension-quake PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-quake PRIVATE siege-extension)

    add_library(siege-extension-hexen-2 SHARED src/sof.cpp src/id-tech-2.def)
    set_target_properties(siege-extension-hexen-2 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-hexen-2 PRIVATE siege-extension)

    add_library(siege-extension-quake-2 SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-quake-2 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-quake-2 PRIVATE siege-extension)

    #TODO finish the siege-installation logic for heretic 2 so that it is easy to extend
 #   add_library(siege-extension-heretic-2 SHARED src/sof.cpp src/id-tech-2.5.def)
 #   set_target_properties(siege-extension-heretic-2 PROPERTIES CXX_STANDARD 23)
 #   target_link_libraries(siege-extension-heretic-2 PRIVATE siege-extension)

    add_library(siege-extension-soldier-of-fortune SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-soldier-of-fortune PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-soldier-of-fortune PRIVATE siege-extension)

    add_library(siege-extension-anachronox SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-anachronox PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-anachronox PRIVATE siege-extension)

    add_library(siege-extension-daikatana SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-daikatana PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-daikatana PRIVATE siege-extension)

    add_library(siege-extension-kingpin SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-kingpin PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-kingpin PRIVATE siege-extension)

    add_library(siege-extension-sin SHARED src/sof.cpp src/id-tech-2.5.def)
    set_target_properties(siege-extension-sin PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-sin PRIVATE siege-extension)

    add_library(siege-extension-quake-3 SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-quake-3 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-quake-3 PRIVATE siege-extension)

    add_library(siege-extension-return-to-castle-wolf SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-return-to-castle-wolf PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-return-to-castle-wolf PRIVATE siege-extension)

    add_library(siege-extension-elite-force SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-elite-force PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-elite-force PRIVATE siege-extension)

    add_library(siege-extension-elite-force-2 SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-elite-force-2 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-elite-force-2 PRIVATE siege-extension)

    add_library(siege-extension-jedi-outcast SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-jedi-outcast PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-jedi-outcast PRIVATE siege-extension)

    add_library(siege-extension-jedi-academy SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-jedi-academy PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-jedi-academy PRIVATE siege-extension)

    add_library(siege-extension-soldier-of-fortune-2 SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-soldier-of-fortune-2 PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-soldier-of-fortune-2 PRIVATE siege-extension)

    add_library(siege-extension-allied-assault SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-allied-assault PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-allied-assault PRIVATE siege-extension)

    add_library(siege-extension-call-of-duty SHARED src/sof.cpp src/id-tech-3.def)
    set_target_properties(siege-extension-call-of-duty PROPERTIES CXX_STANDARD 23)
    target_link_libraries(siege-extension-call-of-duty PRIVATE siege-extension)
endif()
