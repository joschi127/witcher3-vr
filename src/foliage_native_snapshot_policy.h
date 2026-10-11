#pragma once

#include <cstdint>

#include "shader_fingerprints.h"

namespace w3vr::foliage_native_snapshot {

[[nodiscard]] constexpr bool shader_pair_is_owner(
    uint64_t vs_hash,
    uint64_t ps_hash) {
    using shader_fingerprints::Id;
    using shader_fingerprints::matches;
    return
        (matches(Id::DistantTreesVs, vs_hash) &&
            matches(Id::DistantTreesPs, ps_hash)) ||
        (matches(Id::NearFrondsVs, vs_hash) && matches(Id::NearFrondsPs, ps_hash));
}

[[nodiscard]] constexpr bool is_owner(
    uint64_t vs_hash,
    uint64_t ps_hash,
    bool has_hull_shader,
    bool has_domain_shader,
    bool has_geometry_shader,
    bool triangle_topology,
    bool depth_enabled) {
    return shader_pair_is_owner(vs_hash, ps_hash) &&
        !has_hull_shader && !has_domain_shader && !has_geometry_shader &&
        triangle_topology && depth_enabled;
}

[[nodiscard]] constexpr bool route_active(
    bool openxr_enabled,
    bool hmd_freelook) {
    return openxr_enabled && hmd_freelook;
}

}  // namespace w3vr::foliage_native_snapshot
