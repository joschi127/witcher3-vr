#include "shader_fingerprints.h"

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
    using namespace w3vr::shader_fingerprints;

    require(kEntryCount == 29, "29 fingerprints");
    for (size_t index = 0; index < kEntryCount; ++index) {
        const auto& e = kEntries[index];
        require(e.next_gen_404 != 0, "every fingerprint has a 4.04 hash");
        const auto old_lookup = find(e.next_gen_404);
        require(old_lookup.index == static_cast<int>(index) &&
                old_lookup.build == Build::NextGen404,
            "4.04 hash is unique and found at its own index");
        if (e.remaster_501 != 0) {
            const auto new_lookup = find(e.remaster_501);
            require(new_lookup.index == static_cast<int>(index) &&
                    new_lookup.build == Build::Remaster501,
                "5.01 hash is unique and found at its own index");
            require(e.remaster_501 != e.next_gen_404,
                "builds share no shader hash");
        }
        require(matches(e.id, e.next_gen_404), "4.04 hash matches its id");
        require(!matches(e.id, 0), "zero never matches");
        require(e.name != nullptr && std::strlen(e.name) != 0, "entry has a name");
        require(e.group != nullptr && std::strlen(e.group) != 0, "entry has a group");
        for (size_t other = index + 1; other < kEntryCount; ++other) {
            require(std::strcmp(e.name, kEntries[other].name) != 0,
                "names are unique");
        }
    }

    require(matches(Id::RealSmokeVs, hash_for(Id::RealSmokeVs, Build::Remaster501)),
        "a mapped 5.01 hash matches its id");
    require(!matches(Id::RealSmokePs, hash_for(Id::RealSmokeVs, Build::NextGen404)),
        "a hash does not match another id");
    require(hash_for(Id::TaauResolveCs, Build::Remaster501) == 0,
        "TAAU is unmapped on 5.01");
    require(find(0).index == -1, "zero hash is unknown");
    require(find(0x0123456789ABCDEFull).index == -1, "unknown hash is unknown");
    require(std::strcmp(stage_name(Stage::Compute), "cs") == 0, "stage names");
    require(std::strcmp(build_name(Build::Remaster501), "5.01") == 0, "build names");

    static_assert(matches(Id::TaauResolveCs, 0x9FBCF9A46B050321ull),
        "lookup is usable at compile time");

    if (failures != 0) {
        std::fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    std::puts("shader_fingerprints tests passed");
    return 0;
}
