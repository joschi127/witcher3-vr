if(NOT DEFINED DXGI_PROXY_SOURCE OR NOT EXISTS "${DXGI_PROXY_SOURCE}")
    message(FATAL_ERROR "DXGI proxy source is required")
endif()

file(READ "${DXGI_PROXY_SOURCE}" source)

foreach(fragment IN ITEMS
        "std::atomic<bool> g_hmd_f9_latched{};"
        "int recenter_virtual_key{VK_F9};"
        "(GetAsyncKeyState(g_config.recenter_virtual_key) & 0x8000) != 0"
        "!g_hmd_f9_latched.exchange(true, std::memory_order_relaxed)"
        "g_hmd_f9_latched.store(false, std::memory_order_relaxed);"
        "if (manual_recenter || !g_hmd_center_valid.load())")
    string(FIND "${source}" "${fragment}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing F9 physical key-edge contract: ${fragment}")
    endif()
endforeach()

string(FIND "${source}" "GetAsyncKeyState(VK_F9) & 1" obsolete)
string(FIND "${source}" "GetAsyncKeyState(g_config.recenter_virtual_key) & 1"
    obsolete_configured)
if(NOT obsolete EQUAL -1 OR NOT obsolete_configured EQUAL -1)
    message(FATAL_ERROR "Unreliable F9 low-bit polling remains")
endif()

message(STATUS "F9 physical key-edge contract verified")
