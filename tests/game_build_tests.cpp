#include "engine_addresses.h"
#include "game_build.h"

#include <cstdio>
#include <cstring>

namespace {
int failures{};
void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}
}

int main() {
    using namespace w3vr::game_build;

    require(identify(kNextGen404.time_date_stamp, kNextGen404.size_of_image) ==
            Build::NextGen404,
        "4.04 PE identity is recognized");
    require(identify(kRemaster501.time_date_stamp, kRemaster501.size_of_image) ==
            Build::Remaster501,
        "Remaster 5.01 PE identity is recognized");
    require(identify(kNextGen404.time_date_stamp, kRemaster501.size_of_image) ==
            Build::Unknown,
        "mixed identity fields are unknown");
    require(identify(0, 0) == Build::Unknown, "empty identity is unknown");

    require(engine_hooks_supported(Build::NextGen404),
        "engine hooks stay enabled on 4.04");
    require(!engine_hooks_supported(Build::Remaster501),
        "engine hooks are disabled on the Remaster until ported");
    require(!engine_hooks_supported(Build::Unknown),
        "engine hooks are disabled on unknown builds");

    require(std::strcmp(name(Build::NextGen404), "next-gen-4.04") == 0,
        "4.04 build name");
    require(std::strcmp(name(Build::Unknown), "unknown") == 0,
        "unknown build name");

    static_assert(identify(kNextGen404.time_date_stamp,
                      kNextGen404.size_of_image) == Build::NextGen404);

    {
        namespace addresses = w3vr::engine_addresses;
        using addresses::Function;
        require(addresses::rva(Build::NextGen404, Function::IsAnyMenu) == 0x01F3C5C0,
            "4.04 IsAnyMenu keeps its original RVA");
        require(addresses::rva(Build::Remaster501, Function::IsAnyMenu) == 0x01DF41A0,
            "5.01 IsAnyMenu RVA");
        require(addresses::rva(Build::NextGen404, Function::IsLoadingScreenVideoPlaying) ==
                0x0154EED0,
            "4.04 IsLoadingScreenVideoPlaying keeps its original RVA");
        require(addresses::rva(Build::Remaster501, Function::IsLoadingScreenVideoPlaying) ==
                0x0227A0F0,
            "5.01 IsLoadingScreenVideoPlaying RVA");
        require(addresses::rva(Build::Unknown, Function::IsAnyMenu) == 0,
            "no engine addresses on unknown builds");
        require(addresses::signature(Build::NextGen404, Function::IsAnyMenu).size == 0,
            "4.04 entries are not signature-checked");
        const auto remaster =
            addresses::signature(Build::Remaster501, Function::IsAnyMenu);
        require(remaster.size != 0, "5.01 entries carry a signature");
        require(addresses::signature_matches(remaster.bytes, remaster),
            "a signature matches its own bytes");
        uint8_t changed[64]{};
        std::memcpy(changed, remaster.bytes, remaster.size);
        changed[remaster.size - 1] ^= 0xFF;
        require(!addresses::signature_matches(changed, remaster),
            "a changed byte fails the signature");
        require(addresses::signature_matches(changed, addresses::Signature{nullptr, 0}),
            "an empty signature always matches");
    }

    if (failures == 0) {
        std::puts("game_build tests passed");
    }
    return failures == 0 ? 0 : 1;
}
