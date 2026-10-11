#include "foliage_native_snapshot_policy.h"

#include <cassert>

int main() {
    using namespace w3vr::foliage_native_snapshot;
    using w3vr::shader_fingerprints::Build;
    using w3vr::shader_fingerprints::Id;
    using w3vr::shader_fingerprints::hash_for;

    constexpr auto kDistantTreesVsHash = hash_for(Id::DistantTreesVs, Build::NextGen404);
    constexpr auto kDistantTreesPsHash = hash_for(Id::DistantTreesPs, Build::NextGen404);
    constexpr auto kNearFrondsVsHash = hash_for(Id::NearFrondsVs, Build::NextGen404);
    constexpr auto kNearFrondsPsHash = hash_for(Id::NearFrondsPs, Build::NextGen404);

    static_assert(shader_pair_is_owner(
        kDistantTreesVsHash, kDistantTreesPsHash));
    static_assert(shader_pair_is_owner(kNearFrondsVsHash, kNearFrondsPsHash));
    static_assert(!shader_pair_is_owner(
        kDistantTreesVsHash, kNearFrondsPsHash));
    static_assert(shader_pair_is_owner(
        hash_for(Id::NearFrondsVs, Build::Remaster501),
        hash_for(Id::NearFrondsPs, Build::Remaster501)));
    static_assert(is_owner(
        kNearFrondsVsHash, kNearFrondsPsHash,
        false, false, false, true, true));
    static_assert(!is_owner(
        kNearFrondsVsHash, kNearFrondsPsHash,
        false, false, true, true, true));
    static_assert(route_active(true, true));
    static_assert(!route_active(true, false));
    return 0;
}
