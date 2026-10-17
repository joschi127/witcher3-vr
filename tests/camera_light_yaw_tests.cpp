#include "camera_light_yaw.h"

#include <cstdio>

namespace {
int failures{};
void require(bool condition, const char* message) {
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}
}  // namespace

int main() {
    using w3vr::camera_light_yaw::Ledger;

    Ledger ledger;
    float base{};
    require(!ledger.lookup(12.5f, base), "empty ledger misses");

    ledger.record(32.5f, 12.5f);
    require(ledger.lookup(32.5f, base) && base == 12.5f,
        "composed yaw maps to the game yaw");
    require(!ledger.lookup(32.50001f, base), "only exact composed yaw matches");

    ledger.record(-170.0f, -170.0f);
    require(!ledger.lookup(-170.0f, base), "unchanged yaw is not recorded");

    for (int i = 0; i < 100; ++i) {
        ledger.record(1000.0f + static_cast<float>(i), static_cast<float>(i));
    }
    require(!ledger.lookup(32.5f, base), "old entries are overwritten");
    require(ledger.lookup(1099.0f, base) && base == 99.0f,
        "newest entry is found");

    if (failures == 0) {
        std::puts("camera_light_yaw tests passed");
    }
    return failures == 0 ? 0 : 1;
}
