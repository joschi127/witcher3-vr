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

    if (failures == 0) {
        std::puts("game_build tests passed");
    }
    return failures == 0 ? 0 : 1;
}
