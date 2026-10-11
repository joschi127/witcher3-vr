#pragma once

#include <cstddef>
#include <cstdint>

// Every shader the renderer code recognizes, with its fingerprint (FNV-1a 64
// of the full bytecode) per supported game build. Call sites compare through
// matches(), which accepts the hash of any build: the builds share no shader
// hashes, so no build switch is needed. A zero hash means "not mapped for this
// build"; the feature then never engages there and keeps the native draw.
//
// Remaster 5.01 recompiled every shader. Its hashes were matched offline
// against 4.04 (signatures, resource bindings, DXIL op histograms and whole
// pipeline combinations). A 5.01 hash is only filled in when the feature's
// other assumptions (root layout, constant-buffer offsets, signatures) were
// also checked; see docs/remaster-porting-analysis.md.
namespace w3vr::shader_fingerprints {

enum class Stage : uint8_t { Vertex, Hull, Domain, Pixel, Compute };

enum class Id : uint8_t {
    NativeTemporalTerrainVs,
    NativeTemporalTerrainHs,
    NativeTemporalTerrainSecondHs,
    NativeTemporalTerrainDs,
    RealSmokeVs,
    RealSmokePs,
    TaauResolveCs,
    TaauMotionWriterVs,
    TaauMotionWriterPs,
    TiledLightResolveCs,
    TiledDimmerCullCs,
    RtAoHistoryReaderCs,
    RtAoHistoryWriterCs,
    RtShadowPreBlurCs,
    RtShadowTemporalStabilizationCs,
    RtSpecularTemporalAccumulationCs,
    RtSpecularTemporalStabilizationCs,
    NudgeProbeAVs,
    NudgeProbeAPs,
    NudgeProbeBVs,
    NudgeProbeBPs,
    DistantTreesVs,
    DistantTreesPs,
    NearFrondsVs,
    NearFrondsPs,
    HudCompositeVs,
    HudCompositePs,
    AimProbeVs,
    AimProbePs,
    Count,
};

enum class Build : uint8_t { NextGen404, Remaster501 };

struct Entry {
    Id id;
    Stage stage;
    const char* group;
    const char* name;
    uint64_t next_gen_404;
    uint64_t remaster_501;
};

// clang-format off
inline constexpr Entry kEntries[] = {
    {Id::NativeTemporalTerrainVs, Stage::Vertex, "terrain", "native_temporal_terrain_vs",
        0x5D723F550153F88Bull, 0xDE906E0F02EC346Cull},
    {Id::NativeTemporalTerrainHs, Stage::Hull, "terrain", "native_temporal_terrain_hs",
        0x5B33D68BABD52A7Eull, 0xCB844D37FC509BB2ull},
    {Id::NativeTemporalTerrainSecondHs, Stage::Hull, "terrain", "native_temporal_terrain_second_hs",
        0x9ABE7F60D2CFC2EBull, 0x256DC74B40DCADB5ull},
    {Id::NativeTemporalTerrainDs, Stage::Domain, "terrain", "native_temporal_terrain_ds",
        0x4793F519D8E9F837ull, 0xA061EACD9609C420ull},
    {Id::RealSmokeVs, Stage::Vertex, "smoke", "real_smoke_vs",
        0x840EF7AC5229BC58ull, 0x129AFB428823EC6Dull},
    {Id::RealSmokePs, Stage::Pixel, "smoke", "real_smoke_ps",
        0x88504F5A627B9F03ull, 0x1BD15D405C024C59ull},
    // TAAU no longer exists in 5.01.
    {Id::TaauResolveCs, Stage::Compute, "taau", "taau_resolve_cs",
        0x9FBCF9A46B050321ull, 0},
    {Id::TaauMotionWriterVs, Stage::Vertex, "taau", "taau_motion_writer_vs",
        0x9503CB0CCE8D37AFull, 0},
    {Id::TaauMotionWriterPs, Stage::Pixel, "taau", "taau_motion_writer_ps",
        0x835BD7C8E1E8FD93ull, 0},
    // 5.01 moved the patched SharedPixelConsts offsets; map after the code
    // handles the 5.01 layout.
    {Id::TiledLightResolveCs, Stage::Compute, "tiled_light", "tiled_light_resolve_cs",
        0x2F58A01A6E3F1800ull, 0},
    {Id::TiledDimmerCullCs, Stage::Compute, "tiled_light", "tiled_dimmer_cull_cs",
        0x9D732288E451D8CAull, 0},
    // Ray-traced history passes: not yet compared on 5.01 (RT was off).
    {Id::RtAoHistoryReaderCs, Stage::Compute, "rt", "rt_ao_history_reader_cs",
        0xB023FC3C1198AD08ull, 0},
    {Id::RtAoHistoryWriterCs, Stage::Compute, "rt", "rt_ao_history_writer_cs",
        0x060788714B3B371Bull, 0},
    {Id::RtShadowPreBlurCs, Stage::Compute, "rt", "rt_shadow_pre_blur_cs",
        0x64C9E098A7C07F3Aull, 0},
    {Id::RtShadowTemporalStabilizationCs, Stage::Compute, "rt", "rt_shadow_temporal_stabilization_cs",
        0x365D2AB2D6ED60B2ull, 0},
    {Id::RtSpecularTemporalAccumulationCs, Stage::Compute, "rt", "rt_specular_temporal_accumulation_cs",
        0x208C44DC77E1FAD9ull, 0},
    {Id::RtSpecularTemporalStabilizationCs, Stage::Compute, "rt", "rt_specular_temporal_stabilization_cs",
        0x723416A8E26035B8ull, 0},
    // Reverse-engineering probes; the 5.01 pixel shaders are still ambiguous.
    {Id::NudgeProbeAVs, Stage::Vertex, "probe", "nudge_probe_a_vs",
        0xEDFD76797EB58077ull, 0},
    {Id::NudgeProbeAPs, Stage::Pixel, "probe", "nudge_probe_a_ps",
        0xBB5967B70E8594BFull, 0},
    {Id::NudgeProbeBVs, Stage::Vertex, "probe", "nudge_probe_b_vs",
        0xA6D5EB4C9688AB1Bull, 0},
    {Id::NudgeProbeBPs, Stage::Pixel, "probe", "nudge_probe_b_ps",
        0xA04F949CEF867EA0ull, 0},
    {Id::DistantTreesVs, Stage::Vertex, "foliage", "distant_trees_vs",
        0x5E2E73E55B072A74ull, 0x11A19E4F09BC2821ull},
    {Id::DistantTreesPs, Stage::Pixel, "foliage", "distant_trees_ps",
        0x7FC495F2BB36CAC0ull, 0x8AD1761E3F762C05ull},
    {Id::NearFrondsVs, Stage::Vertex, "foliage", "near_fronds_vs",
        0xF9282625E62BCC6Aull, 0x2D62BEEEBB246012ull},
    {Id::NearFrondsPs, Stage::Pixel, "foliage", "near_fronds_ps",
        0x5B6F5C6CA86B8C9Dull, 0x6751E8F5A337D737ull},
    // 5.01 draws the composite into two render targets (4.04: one); the
    // replacement pipeline needs review before this is mapped.
    {Id::HudCompositeVs, Stage::Vertex, "hud", "hud_composite_vs",
        0x84F8A283C01E56D2ull, 0},
    {Id::HudCompositePs, Stage::Pixel, "hud", "hud_composite_ps",
        0x051248978BCC3943ull, 0},
    {Id::AimProbeVs, Stage::Vertex, "hud", "aim_probe_vs",
        0xA1EFA1FD7AF9F39Full, 0x9AF43734B8441E7Dull},
    {Id::AimProbePs, Stage::Pixel, "hud", "aim_probe_ps",
        0x383D662D669D2C2Cull, 0x0163DE7F3005C9C7ull},
};
// clang-format on

inline constexpr size_t kEntryCount = sizeof(kEntries) / sizeof(kEntries[0]);
static_assert(kEntryCount == static_cast<size_t>(Id::Count),
    "one table entry per fingerprint id");
static_assert(kEntryCount <= 64, "census tracks sightings in a 64-bit mask");

constexpr bool entries_follow_id_order() {
    for (size_t index = 0; index < kEntryCount; ++index) {
        if (static_cast<size_t>(kEntries[index].id) != index) {
            return false;
        }
    }
    return true;
}
static_assert(entries_follow_id_order(), "kEntries must be ordered by Id");

constexpr const Entry& entry(Id id) {
    return kEntries[static_cast<size_t>(id)];
}

constexpr uint64_t hash_for(Id id, Build build) {
    return build == Build::NextGen404 ? entry(id).next_gen_404 :
                                        entry(id).remaster_501;
}

constexpr bool matches(Id id, uint64_t hash) {
    const Entry& e = entry(id);
    return hash != 0 && (hash == e.next_gen_404 || hash == e.remaster_501);
}

struct Lookup {
    int index;
    Build build;
};

// Table index and build of a known hash, or index -1.
constexpr Lookup find(uint64_t hash) {
    if (hash != 0) {
        for (size_t index = 0; index < kEntryCount; ++index) {
            if (kEntries[index].next_gen_404 == hash) {
                return {static_cast<int>(index), Build::NextGen404};
            }
            if (kEntries[index].remaster_501 == hash) {
                return {static_cast<int>(index), Build::Remaster501};
            }
        }
    }
    return {-1, Build::NextGen404};
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

constexpr const char* build_name(Build build) {
    return build == Build::NextGen404 ? "4.04" : "5.01";
}

}  // namespace w3vr::shader_fingerprints
