if(NOT DEFINED DXGI_PROXY_SOURCE OR NOT EXISTS "${DXGI_PROXY_SOURCE}")
    message(FATAL_ERROR "DXGI proxy source is required")
endif()

file(READ "${DXGI_PROXY_SOURCE}" source)

# All renderer-owned function-key inputs use physical-down state. Other
# modules (OptiScaler/ReShade/game) may independently consume the low bit.
# F9 (recenter) is configurable via [openxr] recenter_key; its sampling is
# covered by f9_key_edge_contract.cmake.
foreach(key IN ITEMS F2 F3 F6 F8 F10 F11)
    string(FIND "${source}" "(GetAsyncKeyState(VK_${key}) & 0x8000) != 0" physical)
    if(physical EQUAL -1)
        message(FATAL_ERROR "${key} lacks physical-down sampling")
    endif()
    string(FIND "${source}" "GetAsyncKeyState(VK_${key}) & 1" low_bit)
    if(NOT low_bit EQUAL -1)
        message(FATAL_ERROR "${key} still consumes the unreliable low bit")
    endif()
endforeach()

foreach(fragment IN ITEMS
        "std::atomic<bool> g_puredark_afw_f6_latched{};"
        "!g_puredark_afw_f6_latched.exchange(true, std::memory_order_relaxed)"
        "g_puredark_afw_f6_latched.store(false, std::memory_order_relaxed);"
        "const bool ctrl_down = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;"
        "const bool chord_down = f6_down && ctrl_down;"
        "if (!chord_pressed)"
        "g_f2_hotkey_down.exchange("
        "g_f3_hotkey_down.exchange("
        "g_close_camera_f8_latched.exchange(true, std::memory_order_relaxed)"
        "g_hmd_f9_latched.exchange(true, std::memory_order_relaxed)"
        "g_manual_cinema_f10_latched.exchange(true, std::memory_order_relaxed)"
        "g_first_person_f11_latched.exchange(true, std::memory_order_relaxed)")
    string(FIND "${source}" "${fragment}" present)
    if(present EQUAL -1)
        message(FATAL_ERROR "Key-edge contract missing: ${fragment}")
    endif()
endforeach()

message(STATUS "All renderer-owned function-key edges verified")
