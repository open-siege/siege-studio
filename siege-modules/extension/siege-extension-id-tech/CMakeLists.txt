cmake_minimum_required(VERSION 3.30)
project(siege-extension-id-tech)

if (WIN32)
    add_library(siege-extension-id-tech-2-0 STATIC src/id-tech-shared.cpp src/cmd-id-tech-2-0.cpp)
    target_include_directories(siege-extension-id-tech-2-0 PUBLIC src)
    
    add_library(siege-extension-take-no-prisoners SHARED src/take-no-prisoners.cpp src/vampire.def)
    add_library(siege-extension-mageslayer SHARED src/mageslayer.cpp src/mageslayer.def)

    set(VAMPIRE_TARGETS siege-extension-take-no-prisoners 
                        siege-extension-mageslayer) 

    foreach(EXT_TARGET IN ITEMS ${ID_TECH_2_0_TARGETS})
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension-id-tech-2-0)
    endforeach()

    
    add_library(siege-extension-quake SHARED src/quake.cpp src/id-tech-2.def)
    add_library(siege-extension-hexen-2 SHARED src/hexen-2.cpp src/id-tech-2.def)
    add_library(siege-extension-hexen-2-praevus SHARED src/hexen-2-praevus.cpp src/hexen-2-praevus.def)
    add_library(siege-extension-cia-operative SHARED src/cia-operative.cpp src/id-tech-2.def)
    add_library(siege-extension-laser-arena SHARED src/laser-arena.cpp src/id-tech-2.def)
    add_library(siege-extension-battle-metal SHARED src/battle-metal.cpp src/id-tech-2.def)
    
    set(ID_TECH_2_0_TARGETS siege-extension-quake 
                            siege-extension-hexen-2 
                            siege-extension-hexen-2-praevus 
                            siege-extension-cia-operative
                            siege-extension-laser-arena
                            siege-extension-battle-metal) 

    foreach(EXT_TARGET IN ITEMS ${ID_TECH_2_0_TARGETS})
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension-id-tech-2-0)
    endforeach()

    add_library(siege-extension-id-tech-2-5 STATIC src/id-tech-shared.cpp src/cmd-id-tech-2-5.cpp)
    target_include_directories(siege-extension-id-tech-2-5 PUBLIC src)

    add_library(siege-extension-anachronox SHARED src/anox.cpp src/anox.def)
    add_library(siege-extension-quake-2 SHARED src/quake-2.cpp src/id-tech-2.5.def)
    add_library(siege-extension-heretic-2 SHARED src/heretic-2.cpp src/heretic-2.def)
    add_library(siege-extension-daikatana SHARED src/daikatana.cpp src/id-tech-2.5.def)
    add_library(siege-extension-kingpin SHARED src/kingpin.cpp src/id-tech-2.5.def)
    add_library(siege-extension-sin SHARED src/sin.cpp src/id-tech-2.5.def)
    add_library(siege-extension-soldier-of-fortune SHARED src/sof.cpp src/id-tech-2.5.def)

    set(ID_TECH_2_5_TARGETS siege-extension-anachronox 
                            siege-extension-quake-2
                            siege-extension-heretic-2
                            siege-extension-daikatana
                            siege-extension-kingpin
                            siege-extension-sin
                            siege-extension-soldier-of-fortune
    ) 
    
    
    foreach(EXT_TARGET IN ITEMS ${ID_TECH_2_5_TARGETS})
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension-id-tech-2-5)
    endforeach()

    add_library(siege-extension-id-tech-3-0 STATIC src/id-tech-shared.cpp src/cmd-id-tech-3-0.cpp)
    target_include_directories(siege-extension-id-tech-3-0 PUBLIC src)

    add_library(siege-extension-return-to-castle-wolf-sp SHARED src/wolf-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-return-to-castle-wolf-mp SHARED src/wolf-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-wolf-et SHARED src/wolf-et.cpp src/id-tech-3.def)
    add_library(siege-extension-elite-force-sp SHARED src/elite-force-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-elite-force-mp SHARED src/elite-force-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-elite-force-2 SHARED src/elite-force-2.cpp  src/id-tech-3.def)
    add_library(siege-extension-jedi-outcast-sp SHARED src/jedi-outcast-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-jedi-academy-sp SHARED src/jedi-academy-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-jedi-outcast-mp SHARED src/jedi-outcast-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-jedi-academy-mp SHARED src/jedi-academy-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-soldier-of-fortune-2-sp SHARED src/sof-2-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-soldier-of-fortune-2-mp SHARED src/sof-2-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-call-of-duty-sp SHARED src/call-of-duty-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-call-of-duty-uo-sp SHARED src/call-of-duty-uo-sp.cpp src/id-tech-3.def)
    add_library(siege-extension-call-of-duty-mp SHARED src/call-of-duty-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-call-of-duty-uo-mp SHARED src/call-of-duty-uo-mp.cpp src/id-tech-3.def)
    add_library(siege-extension-quake-3 SHARED src/quake-3.cpp src/id-tech-3.def)
    add_library(siege-extension-quake-live SHARED src/quake-live.cpp src/id-tech-3.def)
    add_library(siege-extension-space-trader SHARED src/space-trader.cpp src/space-trader.def)
    add_library(siege-extension-iron-grip SHARED src/iron-grip.cpp src/id-tech-3.def)
    
    add_library(siege-extension-fakk-2 SHARED src/fakk-2.cpp src/fakk-2.def)
    add_library(siege-extension-alice SHARED src/alice.cpp src/id-tech-3.def)
    add_library(siege-extension-allied-assault SHARED src/allied-assault.cpp src/id-tech-3.def)
    add_library(siege-extension-allied-assault-breakthrough SHARED src/allied-assault-breakthrough.cpp src/id-tech-3.def)
    add_library(siege-extension-allied-assault-spearhead SHARED src/allied-assault-spearhead.cpp src/id-tech-3.def)
    add_library(siege-extension-pacific-assault SHARED src/pacific-assault.cpp src/id-tech-3.def)

    
    get_property(ID_TECH_TARGETS
                DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY BUILDSYSTEM_TARGETS)

    foreach(EXT_TARGET IN ITEMS ${ID_TECH_TARGETS})
        get_target_property(TARGET_TYPE ${EXT_TARGET} TYPE)

        if (${TARGET_TYPE} STREQUAL "STATIC_LIBRARY" OR EXT_TARGET IN_LIST ID_TECH_2_0_TARGETS OR EXT_TARGET IN_LIST ID_TECH_2_5_TARGETS)
            continue()
        endif()
        target_link_libraries(${EXT_TARGET} PRIVATE siege-extension-id-tech-3-0)
    endforeach()

    foreach(EXT_TARGET IN ITEMS ${ID_TECH_TARGETS})
        set_target_properties(${EXT_TARGET} PROPERTIES CXX_STANDARD 23)
	    target_link_libraries(${EXT_TARGET} PRIVATE siege-extension siege-content-id-tech)
    endforeach()
endif()
