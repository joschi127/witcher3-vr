#include "shader_fingerprint_census.h"

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
    using namespace w3vr::shader_fingerprint_census;

    require(kKnownFingerprintCount == 29, "29 known fingerprints");
    for (size_t index = 0; index < kKnownFingerprintCount; ++index) {
        const auto& entry = kKnownFingerprints[index];
        require(entry.hash != 0, "fingerprint hash is non-zero");
        require(find_known(entry.hash) == static_cast<int>(index),
            "fingerprint hashes are unique and found at their own index");
        require(entry.name != nullptr && std::strlen(entry.name) != 0,
            "fingerprint has a name");
        require(entry.group != nullptr && std::strlen(entry.group) != 0,
            "fingerprint has a group");
        for (size_t other = index + 1; other < kKnownFingerprintCount; ++other) {
            require(std::strcmp(entry.name, kKnownFingerprints[other].name) != 0,
                "fingerprint names are unique");
        }
    }
    require(find_known(0) == -1, "zero hash is not a fingerprint");
    require(find_known(0x0123456789ABCDEFull) == -1, "unknown hash is not found");
    require(std::strcmp(stage_name(Stage::Compute), "cs") == 0, "stage names");

    static_assert(find_known(0x9FBCF9A46B050321ull) >= 0,
        "lookup is usable at compile time");

    if (failures != 0) {
        std::fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    std::puts("shader_fingerprint_census tests passed");
    return 0;
}
