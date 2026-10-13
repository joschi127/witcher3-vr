#pragma once

#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <cstring>

// Hook for small engine functions that MinHook cannot patch: 5.01 is an AVX
// build, and MinHook's length disassembler (HDE64) rejects the VEX-encoded
// instructions that such leaf functions start with. The caller supplies the
// exact prefix to relocate: whole instructions, none RIP-relative and none a
// branch target inside the function, verified byte for byte beforehand (the
// per-build signature in engine_addresses.h covers exactly that prefix).
// The prefix is copied into an executable trampoline followed by an absolute
// jump back; the target starts with an absolute jump to the detour.
namespace w3vr::prefix_hook {

// jmp qword ptr [rip+0]; dq destination
inline constexpr size_t kAbsoluteJumpBytes = 14;

inline void write_absolute_jump(uint8_t* at, uint64_t destination) {
    at[0] = 0xFF;
    at[1] = 0x25;
    at[2] = at[3] = at[4] = at[5] = 0x00;
    std::memcpy(at + 6, &destination, sizeof(destination));
}

// Trampoline layout: prefix, then a jump to target + prefix_size.
inline size_t build_trampoline(const uint8_t* prefix, size_t prefix_size,
    uint64_t resume_address, uint8_t* out, size_t out_size) {
    if (prefix_size + kAbsoluteJumpBytes > out_size) {
        return 0;
    }
    std::memcpy(out, prefix, prefix_size);
    write_absolute_jump(out + prefix_size, resume_address);
    return prefix_size + kAbsoluteJumpBytes;
}

// Patch for the target: an absolute jump to the detour, the rest of the
// relocated prefix filled with int3.
inline bool build_patch(size_t prefix_size, uint64_t detour, uint8_t* out) {
    if (prefix_size < kAbsoluteJumpBytes) {
        return false;
    }
    write_absolute_jump(out, detour);
    std::memset(out + kAbsoluteJumpBytes, 0xCC, prefix_size - kAbsoluteJumpBytes);
    return true;
}

// Installs the hook. Must run while no thread executes the target (engine
// hooks are installed at startup, before the script VM runs).
inline bool install(uint8_t* target, size_t prefix_size, void* detour, void** original) {
    constexpr size_t kMaxPrefix = 64;
    if (target == nullptr || detour == nullptr || original == nullptr ||
        prefix_size < kAbsoluteJumpBytes || prefix_size > kMaxPrefix) {
        return false;
    }
    auto* trampoline = static_cast<uint8_t*>(VirtualAlloc(nullptr, 4096,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (trampoline == nullptr) {
        return false;
    }
    if (build_trampoline(target, prefix_size,
            reinterpret_cast<uint64_t>(target) + prefix_size, trampoline, 4096) == 0) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    uint8_t patch[kMaxPrefix]{};
    build_patch(prefix_size, reinterpret_cast<uint64_t>(detour), patch);
    DWORD old_protect{};
    if (!VirtualProtect(target, prefix_size, PAGE_EXECUTE_READWRITE, &old_protect)) {
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return false;
    }
    FlushInstructionCache(GetCurrentProcess(), trampoline, prefix_size + kAbsoluteJumpBytes);
    *original = trampoline;
    std::memcpy(target, patch, prefix_size);
    VirtualProtect(target, prefix_size, old_protect, &old_protect);
    FlushInstructionCache(GetCurrentProcess(), target, prefix_size);
    return true;
}

}  // namespace w3vr::prefix_hook
