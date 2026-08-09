# Safer lwIP RX path for local broadcast storms (shadowed lwipopts.h).
# Do not enable FULLDUPLEX/SEM_PER_THREAD here: libzt never calls
# lwip_socket_thread_init() on game threads, so select/connect null-deref.
if (NOT TARGET zt-static)
    return()
endif()

set(_siege_lwip_dir "${CMAKE_BINARY_DIR}/siege-libzt-lwip")
file(MAKE_DIRECTORY "${_siege_lwip_dir}")

file(READ "${CMAKE_SOURCE_DIR}/libzt/src/lwipopts.h" _siege_lwipopts)
string(REPLACE "\r\n" "\n" _siege_lwipopts "${_siege_lwipopts}")

set(_old
    "#define TCPIP_MBOX_SIZE                 0"
    "#define LWIP_TCPIP_CORE_LOCKING_INPUT   1")
set(_new
    "#define TCPIP_MBOX_SIZE                 256"
    "#define LWIP_TCPIP_CORE_LOCKING_INPUT   0")

list(LENGTH _old _siege_lwip_count)
math(EXPR _siege_lwip_last "${_siege_lwip_count} - 1")
foreach (_siege_lwip_i RANGE ${_siege_lwip_last})
    list(GET _old ${_siege_lwip_i} _siege_lwip_from)
    list(GET _new ${_siege_lwip_i} _siege_lwip_to)
    string(REPLACE "${_siege_lwip_from}" "${_siege_lwip_to}" _siege_lwipopts "${_siege_lwipopts}")
endforeach()

foreach (_siege_lwip_from IN LISTS _old)
    string(FIND "${_siege_lwipopts}" "${_siege_lwip_from}" _siege_lwip_pos)
    if (NOT _siege_lwip_pos EQUAL -1)
        message(FATAL_ERROR "Failed to apply siege lwIP override: ${_siege_lwip_from}")
    endif()
endforeach()

file(WRITE "${_siege_lwip_dir}/lwipopts.h" "${_siege_lwipopts}")

foreach (_siege_lwip_target IN ITEMS lwip_obj lwip_pic libzt_obj zt_pic zt-static zt-shared)
    if (TARGET ${_siege_lwip_target})
        target_include_directories(${_siege_lwip_target} BEFORE PRIVATE "${_siege_lwip_dir}")
        target_compile_definitions(${_siege_lwip_target} PRIVATE MAX_QUEUE_ENTRIES=1024)
    endif()
endforeach()
