#pragma once

#include <cstddef>
#include <cstdint>

#include "game_build.h"

// Per-build addresses of engine-internal functions. The Next-Gen 4.04 values are
// the ones this project was written against. Other builds get an address only
// once it has been located and checked; an address of 0 means "unknown on this
// build", and the hook that needs it stays disabled. Entries for builds other
// than 4.04 also carry the expected leading bytes, verified at runtime before
// the hook is placed, so a wrong address fails closed instead of patching
// unrelated code.
namespace w3vr::engine_addresses {

enum class Function : size_t {
    IsAnyMenu,                    // native theGame.IsAnyMenu (GUI manager)
    IsLoadingScreenVideoPlaying,  // native theGame.IsLoadingScreenVideoPlaying
    ActorIsInGameplayScene,       // native CActor.IsInGameplayScene (+0x2F3)
    ActorIsInNonGameplayCutscene, // native CActor.IsInNonGameplayCutscene (+0x2F2)
    GameIsPlayingNonGameplayScene,  // native theGame.IsCurrentlyPlayingNonGameplayScene
    FrameDataFactory,  // builds a frame (render_context, render_settings, scene_descriptor)
    SceneEnqueue,      // queues a frame as CRenderCommand_RenderScene (render_context, frame)
    Count,
};

struct Signature {
    const uint8_t* bytes;
    size_t size;
};

struct Entry {
    const char* name;
    uintptr_t next_gen_404;
    uintptr_t remaster_501;
    Signature remaster_501_signature;
};

// Whole function: forwards to a GUI-manager virtual (+0x1E0) and stores the
// bool result. Identical in 4.04 and 5.01.
inline constexpr uint8_t kIsAnyMenuBytes[] = {
    0x40, 0x53, 0x48, 0x83, 0xEC, 0x20, 0x48, 0xFF, 0x42, 0x30, 0x49, 0x8B,
    0xD8, 0x48, 0x8B, 0x01, 0xFF, 0x90, 0xE0, 0x01, 0x00, 0x00, 0x48, 0x85,
    0xDB, 0x74, 0x02, 0x88, 0x03, 0x48, 0x83, 0xC4, 0x20, 0x5B, 0xC3};

// Whole function: game +0x198 loading screen, its +0x38 player, player vtable
// +0x18 state, active unless the state is 2. Identical in 4.04 and 5.01.
inline constexpr uint8_t kIsLoadingScreenVideoPlayingBytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0xFF,
    0x42, 0x30, 0x49, 0x8B, 0xF8, 0x48, 0x8B, 0x99, 0x98, 0x01, 0x00, 0x00,
    0x48, 0x85, 0xDB, 0x74, 0x30, 0x48, 0x8B, 0x4B, 0x38, 0x48, 0x85, 0xC9,
    0x74, 0x1D, 0x48, 0x8B, 0x01, 0xFF, 0x50, 0x18, 0x83, 0xF8, 0x02, 0x75};

// Script-native bool getters: bump the stack code pointer, store one byte
// flag of the object if a result is requested, return.
inline constexpr uint8_t kActorIsInGameplaySceneBytes[] = {
    0x48, 0xFF, 0x42, 0x30, 0x4D, 0x85, 0xC0, 0x74, 0x0A, 0x0F, 0xB6, 0x81,
    0xF3, 0x02, 0x00, 0x00, 0x41, 0x88, 0x00, 0xC3};
inline constexpr uint8_t kActorIsInNonGameplayCutsceneBytes[] = {
    0x48, 0xFF, 0x42, 0x30, 0x4D, 0x85, 0xC0, 0x74, 0x0A, 0x0F, 0xB6, 0x81,
    0xF2, 0x02, 0x00, 0x00, 0x41, 0x88, 0x00, 0xC3};
inline constexpr uint8_t kGameIsPlayingNonGameplaySceneBytes[] = {
    0x48, 0xFF, 0x42, 0x30, 0x4D, 0x85, 0xC0, 0x74, 0x0A, 0x0F, 0xB6, 0x81,
    0x1F, 0x01, 0x00, 0x00, 0x41, 0x88, 0x00, 0xC3};

// Large functions differ between the builds (5.01 is an AVX build); their 5.01
// entries carry the 5.01 prologue, which is unique in the 5.01 exe. The match
// itself comes from the call graph: render-scene setup calls the factory, the
// factory calls the view rebuild, and the enqueue is the only other function
// that builds a CRenderCommand_RenderScene with the same logic.
inline constexpr uint8_t kFrameDataFactory501Bytes[] = {
    0x48, 0x89, 0x54, 0x24, 0x10, 0x55, 0x53, 0x57, 0x48, 0x8D, 0xAC, 0x24,
    0xA0, 0x6C, 0xFF, 0xFF};
inline constexpr uint8_t kSceneEnqueue501Bytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B,
    0x05, 0x4F, 0xCF, 0x79};

inline constexpr Entry kEntries[] = {
    {"IsAnyMenu", 0x01F3C5C0, 0x01DF41A0, {kIsAnyMenuBytes, sizeof(kIsAnyMenuBytes)}},
    {"IsLoadingScreenVideoPlaying", 0x0154EED0, 0x0227A0F0,
        {kIsLoadingScreenVideoPlayingBytes, sizeof(kIsLoadingScreenVideoPlayingBytes)}},
    {"Actor.IsInGameplayScene", 0x01C4CD70, 0x0210B410,
        {kActorIsInGameplaySceneBytes, sizeof(kActorIsInGameplaySceneBytes)}},
    {"Actor.IsInNonGameplayCutscene", 0x01C4CD90, 0x0210B430,
        {kActorIsInNonGameplayCutsceneBytes, sizeof(kActorIsInNonGameplayCutsceneBytes)}},
    {"Game.IsPlayingNonGameplayScene", 0x01549800, 0x022753C0,
        {kGameIsPlayingNonGameplaySceneBytes, sizeof(kGameIsPlayingNonGameplaySceneBytes)}},
    {"FrameDataFactory", 0x016214C0, 0x022C0820,
        {kFrameDataFactory501Bytes, sizeof(kFrameDataFactory501Bytes)}},
    {"SceneEnqueue", 0x01621FC0, 0x022BE990,
        {kSceneEnqueue501Bytes, sizeof(kSceneEnqueue501Bytes)}},
};
static_assert(sizeof(kEntries) / sizeof(kEntries[0]) ==
    static_cast<size_t>(Function::Count));

constexpr const Entry& entry(Function function) {
    return kEntries[static_cast<size_t>(function)];
}

// Build-dependent data addresses and object layout values (0 = unknown).
enum class Value : size_t {
    RenderSchedulerGlobal,      // RVA of the render scheduler pointer (flags at +0x3C)
    RenderContextSharedOffset,  // render context field passed on by the scene enqueue
    Count,
};

struct ValueEntry {
    const char* name;
    uintptr_t next_gen_404;
    uintptr_t remaster_501;
};

inline constexpr ValueEntry kValues[] = {
    {"RenderSchedulerGlobal", 0x057F5930, 0x05A5B8F0},
    {"RenderContextSharedOffset", 0x620, 0x5440},
};
static_assert(sizeof(kValues) / sizeof(kValues[0]) == static_cast<size_t>(Value::Count));

constexpr uintptr_t value(game_build::Build build, Value which) {
    const ValueEntry& entry = kValues[static_cast<size_t>(which)];
    switch (build) {
    case game_build::Build::NextGen404:
        return entry.next_gen_404;
    case game_build::Build::Remaster501:
        return entry.remaster_501;
    case game_build::Build::Unknown:
        break;
    }
    return 0;
}

// RVA of |function| on |build|, or 0 when it is unknown there.
constexpr uintptr_t rva(game_build::Build build, Function function) {
    switch (build) {
    case game_build::Build::NextGen404:
        return entry(function).next_gen_404;
    case game_build::Build::Remaster501:
        return entry(function).remaster_501;
    case game_build::Build::Unknown:
        break;
    }
    return 0;
}

// Expected leading bytes at the RVA on |build|; empty when none are checked.
constexpr Signature signature(game_build::Build build, Function function) {
    return build == game_build::Build::Remaster501 ?
        entry(function).remaster_501_signature : Signature{nullptr, 0};
}

constexpr bool signature_matches(const uint8_t* code, Signature expected) {
    for (size_t i = 0; i < expected.size; ++i) {
        if (code[i] != expected.bytes[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace w3vr::engine_addresses
