#pragma once

#include <cstddef>
#include <cstdint>

// Every shader fingerprint (FNV-1a 64 of the full bytecode) the renderer code
// keys on, collected in one place so a diagnostic census can report which of
// them a game build still creates. The values mirror the constants used at
// their call sites in dxgi_proxy.cpp and foliage_native_snapshot_policy.h;
// keep both in sync when a fingerprint is added or re-derived.
namespace w3vr::shader_fingerprint_census {

enum class Stage : uint8_t { Vertex, Hull, Domain, Pixel, Compute };

struct KnownFingerprint {
    uint64_t hash;
    Stage stage;
    const char* group;
    const char* name;
};

inline constexpr KnownFingerprint kKnownFingerprints[] = {
    {0x5D723F550153F88Bull, Stage::Vertex, "terrain", "native_temporal_terrain_vs"},
    {0x5B33D68BABD52A7Eull, Stage::Hull, "terrain", "native_temporal_terrain_hs"},
    {0x9ABE7F60D2CFC2EBull, Stage::Hull, "terrain", "native_temporal_terrain_second_hs"},
    {0x4793F519D8E9F837ull, Stage::Domain, "terrain", "native_temporal_terrain_ds"},
    {0x840EF7AC5229BC58ull, Stage::Vertex, "smoke", "real_smoke_vs"},
    {0x88504F5A627B9F03ull, Stage::Pixel, "smoke", "real_smoke_ps"},
    {0x9FBCF9A46B050321ull, Stage::Compute, "taau", "taau_resolve_cs"},
    {0x9503CB0CCE8D37AFull, Stage::Vertex, "taau", "taau_motion_writer_vs"},
    {0x835BD7C8E1E8FD93ull, Stage::Pixel, "taau", "taau_motion_writer_ps"},
    {0x2F58A01A6E3F1800ull, Stage::Compute, "tiled_light", "tiled_light_resolve_cs"},
    {0x9D732288E451D8CAull, Stage::Compute, "tiled_light", "tiled_dimmer_cull_cs"},
    {0xB023FC3C1198AD08ull, Stage::Compute, "rt", "rt_ao_history_reader_cs"},
    {0x060788714B3B371Bull, Stage::Compute, "rt", "rt_ao_history_writer_cs"},
    {0x64C9E098A7C07F3Aull, Stage::Compute, "rt", "rt_shadow_pre_blur_cs"},
    {0x365D2AB2D6ED60B2ull, Stage::Compute, "rt", "rt_shadow_temporal_stabilization_cs"},
    {0x208C44DC77E1FAD9ull, Stage::Compute, "rt", "rt_specular_temporal_accumulation_cs"},
    {0x723416A8E26035B8ull, Stage::Compute, "rt", "rt_specular_temporal_stabilization_cs"},
    {0xEDFD76797EB58077ull, Stage::Vertex, "probe", "nudge_probe_a_vs"},
    {0xBB5967B70E8594BFull, Stage::Pixel, "probe", "nudge_probe_a_ps"},
    {0xA6D5EB4C9688AB1Bull, Stage::Vertex, "probe", "nudge_probe_b_vs"},
    {0xA04F949CEF867EA0ull, Stage::Pixel, "probe", "nudge_probe_b_ps"},
    {0x5E2E73E55B072A74ull, Stage::Vertex, "foliage", "distant_trees_vs"},
    {0x7FC495F2BB36CAC0ull, Stage::Pixel, "foliage", "distant_trees_ps"},
    {0xF9282625E62BCC6Aull, Stage::Vertex, "foliage", "near_fronds_vs"},
    {0x5B6F5C6CA86B8C9Dull, Stage::Pixel, "foliage", "near_fronds_ps"},
    {0x84F8A283C01E56D2ull, Stage::Vertex, "hud", "hud_composite_vs"},
    {0x051248978BCC3943ull, Stage::Pixel, "hud", "hud_composite_ps"},
    {0xA1EFA1FD7AF9F39Full, Stage::Vertex, "hud", "aim_probe_vs"},
    {0x383D662D669D2C2Cull, Stage::Pixel, "hud", "aim_probe_ps"},
};

inline constexpr size_t kKnownFingerprintCount =
    sizeof(kKnownFingerprints) / sizeof(kKnownFingerprints[0]);
static_assert(kKnownFingerprintCount <= 64,
    "census tracks sightings in a 64-bit mask");

// Index into kKnownFingerprints, or -1 when the hash is not a known fingerprint.
constexpr int find_known(uint64_t hash) {
    for (size_t index = 0; index < kKnownFingerprintCount; ++index) {
        if (kKnownFingerprints[index].hash == hash) {
            return static_cast<int>(index);
        }
    }
    return -1;
}

constexpr const char* stage_name(Stage stage) {
    switch (stage) {
    case Stage::Vertex: return "vs";
    case Stage::Hull: return "hs";
    case Stage::Domain: return "ds";
    case Stage::Pixel: return "ps";
    case Stage::Compute: return "cs";
    }
    return "?";
}

}  // namespace w3vr::shader_fingerprint_census
