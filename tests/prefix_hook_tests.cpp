#include "prefix_hook.h"

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

using Function = long long (*)();
Function g_original{};

long long detour() {
    return g_original() + 100;
}
}  // namespace

int main() {
    using namespace w3vr::prefix_hook;

    uint8_t jump[kAbsoluteJumpBytes]{};
    write_absolute_jump(jump, 0x1122334455667788ull);
    const uint8_t expected_jump[kAbsoluteJumpBytes] = {
        0xFF, 0x25, 0, 0, 0, 0, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    require(std::memcmp(jump, expected_jump, sizeof(jump)) == 0,
        "absolute jump encoding");

    uint8_t patch[20]{};
    require(!build_patch(13, 0x1000, patch), "prefix shorter than the jump is rejected");
    require(build_patch(20, 0x1000, patch) && patch[0] == 0xFF &&
            patch[14] == 0xCC && patch[19] == 0xCC,
        "patch is a jump padded with int3");

    // mov rax, 5 (7 bytes) twice = 14-byte prefix of whole instructions,
    // then add rax, 1; ret. Returns 6 unhooked, 106 hooked.
    const uint8_t code[] = {
        0x48, 0xC7, 0xC0, 0x05, 0x00, 0x00, 0x00,
        0x48, 0xC7, 0xC0, 0x05, 0x00, 0x00, 0x00,
        0x48, 0x83, 0xC0, 0x01,
        0xC3};
    auto* function = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    require(function != nullptr, "executable test buffer");
    if (function != nullptr) {
        std::memcpy(function, code, sizeof(code));
        FlushInstructionCache(GetCurrentProcess(), function, sizeof(code));
        const auto call = reinterpret_cast<Function>(function);
        require(call() == 6, "unhooked function result");
        void* original{};
        require(install(function, 14, reinterpret_cast<void*>(&detour), &original),
            "hook installs");
        g_original = reinterpret_cast<Function>(original);
        require(call() == 106, "hooked call reaches the detour and the original");
        require(g_original() == 6, "trampoline runs the relocated prefix and resumes");
        require(!install(function, 8, reinterpret_cast<void*>(&detour), &original),
            "prefix shorter than the jump is rejected by install");
    }

    if (failures == 0) {
        std::printf("prefix hook tests passed\n");
    }
    return failures == 0 ? 0 : 1;
}
