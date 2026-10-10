#pragma once

#include <cstdint>

// Identifies the running witcher3.exe from its PE header. Every engine-internal
// hook and RVA in this project was derived from one specific build; on any
// other build those addresses point at unrelated code, so engine hooks must
// stay disabled unless the build is recognized as supported.
namespace w3vr::game_build {

enum class Build {
    Unknown,
    NextGen404,   // Next-Gen 4.04 (file version 4.0.0.103190)
    Remaster501,  // Remaster 5.01 (file version 5.0.0.1048522), not yet supported
};

struct PeIdentity {
    uint32_t time_date_stamp;
    uint32_t size_of_image;
};

inline constexpr PeIdentity kNextGen404{0x664FB0DCu, 0x0612E000u};
inline constexpr PeIdentity kRemaster501{0x6AC555A3u, 0x06402000u};

constexpr bool matches(const PeIdentity& identity,
    uint32_t time_date_stamp, uint32_t size_of_image) {
    return identity.time_date_stamp == time_date_stamp &&
        identity.size_of_image == size_of_image;
}

constexpr Build identify(uint32_t time_date_stamp, uint32_t size_of_image) {
    if (matches(kNextGen404, time_date_stamp, size_of_image)) {
        return Build::NextGen404;
    }
    if (matches(kRemaster501, time_date_stamp, size_of_image)) {
        return Build::Remaster501;
    }
    return Build::Unknown;
}

constexpr bool engine_hooks_supported(Build build) {
    return build == Build::NextGen404;
}

constexpr const char* name(Build build) {
    switch (build) {
    case Build::NextGen404:
        return "next-gen-4.04";
    case Build::Remaster501:
        return "remaster-5.01";
    case Build::Unknown:
        break;
    }
    return "unknown";
}

}  // namespace w3vr::game_build
