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
    TopmostCameraObject,  // native CCameraDirector.GetTopmostCameraObject
    ViewRebuild,       // rebuilds a view's matrices from its inputs (float* view)
    GameplayEntry,     // render-thread gameplay frame task; calls the frame builder
    RenderSceneSetup,  // per-tick scene setup; calls the frame factory
    FrameBuilder,      // render-thread frame build (render_context, frame, extra)
    CameraEnableManualControl,  // native CCustomCamera.EnableManualControl
    CameraDirection,   // native CCameraDirector.GetCameraDirection (first of two copies)
    ViewportResolution,  // (fullscreen, resolution setting, int* width, int* height)
    CameraLightPlacement,  // places one camera light from the render camera (pos + yaw)
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

inline constexpr uint8_t kTopmostCameraObject501Bytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0xFF,
    0x42, 0x30, 0x45, 0x33};

inline constexpr uint8_t kViewRebuild501Bytes[] = {
    0x48, 0x8B, 0xC4, 0x55, 0x53, 0x56, 0x57, 0x48, 0x8D, 0xA8, 0x08, 0xFB,
    0xFF, 0xFF, 0x48, 0x81};

inline constexpr uint8_t kGameplayEntry501Bytes[] = {
    0x40, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B, 0x15, 0x7B, 0xBA, 0xB4,
    0x03, 0x48, 0x8B, 0xF9};
inline constexpr uint8_t kRenderSceneSetup501Bytes[] = {
    0x40, 0x55, 0x57, 0x48, 0x8D, 0xAC, 0x24, 0x98, 0x08, 0xFF, 0xFF, 0xB8,
    0x68, 0xF8, 0x00, 0x00};

// Called from the gameplay entry (return 0x01D0C5E1 on 5.01); passes
// frame+0x10 and the input extent at +0xBEC to the temporal writer.
inline constexpr uint8_t kFrameBuilder501Bytes[] = {
    0x48, 0x8B, 0xC4, 0x48, 0x89, 0x50, 0x10, 0x55, 0x53, 0x56, 0x41, 0x54,
    0x41, 0x55, 0x41, 0x56};

// Registered script native; 24 bytes because the first 16 occur 109 times.
inline constexpr uint8_t kCameraEnableManualControl501Bytes[] = {
    0x48, 0x89, 0x5C, 0x24, 0x10, 0x57, 0x48, 0x83, 0xEC, 0x20, 0x48, 0x8B,
    0x42, 0x30, 0x4C, 0x8D, 0x0D, 0x5B, 0x82, 0xC6, 0x03, 0x48, 0x8B, 0xD9};

// Leaf function: inc [rdx+0x30]; vmovss xmm0/1/2, [rcx+0x790/0x794/0x798].
// The signature is exactly the 20-byte prefix the 5.01 prefix hook relocates
// (whole, position-independent instructions; the only branch targets ret).
inline constexpr uint8_t kCameraDirection501Bytes[] = {
    0x48, 0xFF, 0x42, 0x30, 0xC5, 0xFA, 0x10, 0x81, 0x90, 0x07, 0x00, 0x00,
    0xC5, 0xFA, 0x10, 0x89, 0x94, 0x07, 0x00, 0x00};

// Same signature and logic as 4.04 (desktop size from DXGI, then the "WxH"
// setting); 5.01 moved the DXGI part into a helper (0x0228E230).
inline constexpr uint8_t kViewportResolution501Bytes[] = {
    0x48, 0x8B, 0xC4, 0x88, 0x48, 0x08, 0x57, 0x48, 0x81, 0xEC, 0xB0, 0x00,
    0x00, 0x00, 0x48, 0x89};

// (light out, render camera, light params, setup, colour, ...): rotates the
// light offset by the camera's yaw (camera[6]) only and adds camera[0..2].
// Only caller: the frame light list (the one reading DisableAllCameraLights).
inline constexpr uint8_t kCameraLightPlacement501Bytes[] = {
    0x48, 0x8B, 0xC4, 0x48, 0x89, 0x70, 0x10, 0x4C, 0x89, 0x60, 0x18, 0x55,
    0x41, 0x56, 0x41, 0x57, 0x48, 0x8D, 0xA8, 0xF8, 0xFE, 0xFF, 0xFF, 0x48};

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
    {"CameraDirector.GetTopmostCameraObject", 0x0161B9C0, 0x023825D0,
        {kTopmostCameraObject501Bytes, sizeof(kTopmostCameraObject501Bytes)}},
    {"ViewRebuild", 0x015FE550, 0x02293240,
        {kViewRebuild501Bytes, sizeof(kViewRebuild501Bytes)}},
    {"GameplayEntry", 0x01D00260, 0x01D0C570,
        {kGameplayEntry501Bytes, sizeof(kGameplayEntry501Bytes)}},
    {"RenderSceneSetup", 0x01553070, 0x0227D1F0,
        {kRenderSceneSetup501Bytes, sizeof(kRenderSceneSetup501Bytes)}},
    {"FrameBuilder", 0x01D86400, 0x01C1A210,
        {kFrameBuilder501Bytes, sizeof(kFrameBuilder501Bytes)}},
    {"CustomCamera.EnableManualControl", 0x01F580E0, 0x01E0D800,
        {kCameraEnableManualControl501Bytes, sizeof(kCameraEnableManualControl501Bytes)}},
    {"CameraDirector.GetCameraDirection", 0x0161BBC0, 0x023826F0,
        {kCameraDirection501Bytes, sizeof(kCameraDirection501Bytes)}},
    {"ViewportResolution", 0x01563DC0, 0x0228F3B0,
        {kViewportResolution501Bytes, sizeof(kViewportResolution501Bytes)}},
    {"CameraLightPlacement", 0x00718310, 0x0035B500,
        {kCameraLightPlacement501Bytes, sizeof(kCameraLightPlacement501Bytes)}},
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
    ViewCentreIndex,            // view-array float index of centre offset x, y and viewport w, h
    CustomCameraVtable,         // RVA of the CCustomCamera vtable at subobject +0x170
    ViewFactoryReturn,          // return address of the view rebuild call in the factory
    ViewCopyRebuildReturn,      // return address of the view rebuild call in the copy wrapper
    SceneDescriptorBytes,       // bytes of the stack scene descriptor copied for the second eye
    SceneDescriptorMinimumBytes,  // copy fails below this many readable bytes
    GameplayFrameBuilderReturn,   // return address of the frame-builder call in the gameplay entry
    // Frame data built by the factory: two views (core at +0x20 and the
    // secondary offset, stride 0x510 on 4.04 and 0x5E0 on 5.01), each with a
    // centre offset x, y and extent w, h block, then the input extent.
    FrameSecondaryViewOffset,     // secondary view core (primary core is +0x20)
    FramePrimaryCentreOffset,     // primary centre x, y; extent w, h follows at +8
    FrameSecondaryCentreOffset,   // secondary centre x, y; extent w, h follows at +8
    FrameInputExtentOffset,       // render input width, height
    FrameCameraTransformOffset,   // camera transform (row 3 = position); diagnostics only
    CustomCameraManualControlOffset,  // CCustomCamera manual-control flag byte
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
    {"ViewCentreIndex", 0x100, 0x130},
    {"CustomCameraVtable", 0x02D645B0, 0x03810198},
    {"ViewFactoryReturn", 0x0162196D, 0x022C101A},
    {"ViewCopyRebuildReturn", 0x015FF863, 0x02292EA7},
    // The factory reads up to about +0xB765 (4.04) and +0xF714 (5.01).
    {"SceneDescriptorBytes", 0xC000, 0x10000},
    {"SceneDescriptorMinimumBytes", 0xB000, 0xF800},
    {"GameplayFrameBuilderReturn", 0x01D002CF, 0x01D0C5E1},
    // 5.01 values follow the view stride (+0xD0 per view) and match the
    // temporal writer (frame+0x10 +0x4D0/+0xAB0) and the builder (+0xBEC).
    {"FrameSecondaryViewOffset", 0x530, 0x600},
    {"FramePrimaryCentreOffset", 0x420, 0x4E0},
    {"FrameSecondaryCentreOffset", 0x930, 0xAC0},
    {"FrameInputExtentOffset", 0xA4C, 0xBEC},
    // Secondary view +0x30 (view floats 0x0C-0x1B, unchanged below 0xAC).
    {"FrameCameraTransformOffset", 0x560, 0x630},
    // EnableManualControl: the custom-camera fields moved by +0x10 in 5.01.
    {"CustomCameraManualControlOffset", 0x259, 0x269},
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
