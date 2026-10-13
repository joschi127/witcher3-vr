if(NOT DEFINED DXGI_PROXY_SOURCE OR NOT EXISTS "${DXGI_PROXY_SOURCE}")
    message(FATAL_ERROR "DXGI proxy source was not provided")
endif()

file(READ "${DXGI_PROXY_SOURCE}" source)

set(required_fragments
    "PERF:NOAA-DESCRIPTOR-COPY V1441"
    "bool copy_legacy_resource_metadata"
    "if (copy_legacy_resource_metadata)"
    "PERF:NOAA-GPU-VA-CACHE V1441"
    "std::array<GpuVaReadCache, 8>"
    "return safe_copy_asymmetric_authority(source, destination, size)"
    "const bool protect_taau_unmap = taau_runtime_tracking_active()"
    "PERF:NOAA-FENCE-COMPLETION-CACHE V1441"
    "PERF:NOAA-EXECUTE-ROUTES V1441"
    "if (taau_execute_tracking && command_lists != nullptr)"
    "if (!native_temporal_terrain_motion_route_active() ||"
    "D3D12_GPU_DESCRIPTOR_HANDLE base_descriptor) {\n    if (is_reshade_immediate_command_list(command_list))"
    "return;\n    }\n    if (dlss_graphics_state_tracking_active() &&"
    "D3D12_GPU_VIRTUAL_ADDRESS buffer_location) {\n    if (is_reshade_immediate_command_list(command_list))"
    "return;\n    }\n    if ((native_temporal_terrain_motion_route_active() ||\n            real_smoke_gs_root_cbv_route_active()) &&")

foreach(fragment IN LISTS required_fragments)
    string(FIND "${source}" "${fragment}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "Missing V1441 No-AA safe contract: ${fragment}")
    endif()
endforeach()

set(forbidden_fragments
    "g_taau_readback_command_list"
    "g_taau_readback_submitted"
    "g_rs_set_viewports"
    "g_rs_set_scissor_rects")

foreach(fragment IN LISTS forbidden_fragments)
    string(FIND "${source}" "${fragment}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "V1441 retained removed hot-path state: ${fragment}")
    endif()
endforeach()

message(STATUS "V1441 No-AA safe optimization contract verified")
