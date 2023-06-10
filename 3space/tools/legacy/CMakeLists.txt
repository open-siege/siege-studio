project(legacy-scripts)
cmake_minimum_required(VERSION 3.17)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/darkstar-vol-extractor/nuvol.py
                ${CMAKE_CURRENT_SOURCE_DIR}/colony-wars-rsc-extractor/extract-rsc.py
        CONFIGURATIONS Debug Release
        DESTINATION bin)
