# Remaster port: status log

Chronological record of what has been verified. Details live in `remaster-porting-analysis.md` (code
dependencies, runtime findings) and `remaster-research.md` (public information). Raw logs,
reverse-engineering output and machine-specific data stay out of the repository.

## 2026-10-10

### Environment and baseline
- Two game installations side by side on GOG: Next-Gen 4.04b (separate GOTY product) as the
  reference, and Remaster 5.01. Both run without the mod.
- Upstream v0.9.8v2 builds with Visual Studio 2026 (MSVC 14.51) through a local
  `CMakeUserPresets.json` that inherits `release` with the "Visual Studio 18 2026" generator.
  All upstream tests pass.
- Baseline on 4.04 with a Pico 4 over Virtual Desktop (VDXR), Stereo + TAAU, AMD GPU:
  - **Run 1: official release.** Works. No hook or OpenXR errors.
  - **Run 2: self-built `dxgi.dll` from the same commit.** Behaves identically (same OpenXR setup,
    same set of log message kinds, comparable frame pacing).
  - So the local toolchain is a valid base for the port.
- Observations on this setup:
  - VDXR reports the system name "Oculus Quest2" for the Pico 4.
  - With Virtual Desktop's FOV-tangent reduction, both eyes get a symmetric FOV, so asymmetric
    projection has no effect.
  - DLSS/DLAA are unavailable on AMD, so TAAU is the reference AA path on 4.04.

### Settings comparison (fresh defaults 4.04 vs 5.01)
See `remaster-porting-analysis.md` §6. In short:
- TAAU settings are gone in 5.01, `AAMode` semantics changed, and RT plus wider distances
  are on by default.
- The launcher's 4.04 VR profile must not be applied to 5.01.
- For tests, a Remaster "parity" profile is used: shared keys get the 4.04 VR-profile values,
  and new features (RT, ray reconstruction, editor preset detection) are switched off.
  "Configure Settings for VR" should later apply this automatically per build (see roadmap
  in `CLAUDE.md`).

### Build guard (first code change)
- `src/game_build.h` identifies the exe from PE `TimeDateStamp` + `SizeOfImage` (4.04, 5.01,
  unknown). `engine_build_supported()` in `dxgi_proxy.cpp` logs the result once and makes
  all 22 `install_engine_*` / `initialize_engine_*` routines return early on unsupported builds.
  The unit test is `tests/game_build_tests.cpp`.
- Verified on 4.04: `engine_hooks=enabled`, hooks installed as before.
- Verified on 5.01: `engine_hooks=disabled_unsupported_build`. The proxy loads and the game
  runs stably (menu, map, Witcher Senses, free roaming). No engine patches are applied.

### Key finding: command-list interception on 5.01
- On 5.01, D3D12 device, queue and DXGI hooks fire, but **no command-list hook fires**
  (zero draws counted).
- A diagnostic (`[DIAG:COMMAND-LIST-VTABLE-MODULE]` in `dxgi_proxy.cpp`) found the cause:
  - 5.01 uses the bundled D3D12 Agility SDK runtime.
  - Its command lists carry a per-object function table whose entries point directly into the
    GPU driver (AMD `amdxc64.dll`). The detours on the runtime's functions are bypassed.
- Disabling the Agility SDK is not possible: 5.01 then refuses to start ("Support for
  DirectX 12 is required").
- Next step: intercept command lists by patching each list's embedded function table at
  creation/reset. Success criterion: draws and PSO binds appear in the log on 5.01. After
  that, check which of the 28 shader fingerprints still match.

### Script side
- All vanilla script methods wrapped by `support/mod*` still exist in 5.01 (signatures not
  yet compared).
- 5.01 save files carry an extra `.json` with build id and mod list.

### Tooling notes
- `ctest` loads the built `dxgi.dll` (`swapchain_identity_tests`), which writes the HUD frame
  scale into the real `Documents\The Witcher 3\dx12user.settings`. Back up the file or restore
  it after running the tests.
- With `[openxr] enabled=0`, set `render_width`/`render_height` to 0. Otherwise the proxy
  forces the swapchain to the VR per-eye size and the flat image is stretched.

## 2026-10-11

### Embedded command-list tables patched (branch `port/embedded-command-list-table`)
- Mechanism: every command-list hook slot is registered. After `Reset` and at
  `ExecuteCommandLists`, the hook is written into each list's embedded function table. The
  original pointers used by the hooks dispatch per list: the remembered embedded entry for
  embedded-table lists, the MinHook trampoline otherwise. 4.04 lists have a shared vtable and
  keep the old path.
- Verified on 5.01, flat (no OpenXR), AMD GPU: stable, no visible issues, no errors in the log.
  - The dummy list used to place the detours has a shared vtable. The game's lists have
    embedded tables at object + 0x5C8.
  - Patched slots: DrawInstanced, DrawIndexedInstanced, Dispatch, SetPipelineState,
    SetDescriptorHeaps, SetCompute/GraphicsRootSignature, SetCompute/GraphicsRootDescriptorTable,
    SetGraphicsRootConstantBufferView. Embedded entries point into the driver (`amdxc64.dll`),
    except SetDescriptorHeaps, which points to a separate `D3D12Core.dll` function.
  - The embedded `Reset` entry is the detoured runtime function, so the Reset hook fires without
    patching.
  - No list had an entry differing from the first remembered one.
  - Draws are now counted (about 1,900 indexed draws per frame in free roaming, 0 before).
- Verified on 4.04 with the same build (commit `ac8cd7f`), Pico 4 over Virtual Desktop, Stereo +
  TAAU: no table is patched (shared vtables), OpenXR setup, draw counts, log message kinds and
  frame pacing match the earlier reference run. No visible difference.
- Open: compare the 28 shader fingerprints on 5.01. The first recording of a newly created list (before its first Reset/submission) is not yet
  intercepted. Hook `CreateCommandList`/`CreateCommandList1` if that turns out to matter.

### Shader fingerprint census (4.04 vs 5.01)
- New diagnostic: `[DIAG:SHADER-FINGERPRINT-CENSUS]` with a table of the 29 fingerprints in
  `src/shader_fingerprint_census.h`, a per-run shader inventory file and an optional bytecode dump
  (`[debug] shader_inventory_dump=1`). Same route on both builds (main menu, load, outdoor view
  with trees, Witcher Senses, map).
- 4.04: 21/29 known fingerprints seen. Missing: the six RT shaders (RT off) and the TAAU motion
  writer pair (not created on this route).
- 5.01: **0/29.** No shader hash of 4.04 occurs in 5.01; every shader was recompiled (newer DXIL
  with native 16-bit types). Shader and pipeline counts are of the same order, and no bound
  pipeline escaped the creation hooks on either build, so no extra creation path needs hooking.
- Offline matching of the bytecode dumps (signatures, resource bindings, thread groups, DXIL op
  histograms) finds plausible 5.01 counterparts for most of the 21 fingerprints. Several are
  still ambiguous and need a closer check before new hashes go into the code.
- Shared constant buffers grew in 5.01:
  - `CameraShaderConsts` (b6, mirrored by the mod's replacement HLSL): append-only, all 4.04
    offsets unchanged, two matrices and one vector added at the end.
  - `SharedPixelConsts`: unchanged up to 0x7F0. One array there grew by 0x100, so everything
    after it moved by 0x100, and new members were appended. The tiled-culling path checks the
    4.04 size (4608 bytes, 4368 aligned to 256) and therefore falls back safely on 5.01.
  - `CSConstants` (tiled light/dimmer compute): members inserted near the start and in the
    middle, so most offsets moved.
- Next: resolve the ambiguous matches, then list for each feature which constant-buffer offsets
  it reads and whether they moved.
- Central fingerprint table `src/shader_fingerprints.h` (one entry per recognized shader, hash per
  build). All call sites go through `matches()`. 5.01 hashes are mapped where the feature's other
  assumptions were checked: terrain motion replay, smoke, foliage snapshot, aim probe. Not yet
  mapped: HUD composite (two render targets), tiled culling (moved offsets), nudge probes
  (ambiguous), RT history (not compared), TAAU (removed in 5.01). Unverified at runtime; the
  census logs `build=5.01` sightings.

### Test mode (unattended runs)
- `[test]` input scripts, gamepad recording and PNG captures work on 4.04: the smoke script
  (`scripts/test-input/smoke-continue.txt`) skips the intros, continues the latest save, walks,
  uses Witcher Senses, opens the game menu and quits, without anyone at the controller.
- Flat stereo without OpenXR (`mode=3`, `[openxr] enabled=0`) renders both eyes alternately;
  two consecutive captures show the two eyes with the expected horizontal offset.
- **5.01 reads the gamepad through GameInput** (the game loads `GameInputRedist.dll`; the exe also
  references ScePad). It never calls `XInputGetState` (0 calls in a full run), so the XInput hook
  sees nothing: test scripts cannot drive 5.01 yet, and the first-person snap-turn/head-follow
  input remap (XInput) has no effect there.
- Test mode now also hooks GameInput on 5.01 (`GameInputInitialize` → IGameInput v3
  `GetCurrentReading`/`GetNextReading` → `IGameInputReading::GetGamepadState`). Verified: with a
  connected gamepad every reading goes through the hook and the game switches to gamepad prompts.
  Without a connected gamepad GameInput returns no readings, so 5.01 runs need a controller.
- 5.01 smoke script (`scripts/test-input/smoke-continue-remaster.txt`, derived from a manual
  5.01 recording) runs unattended with a switched-on gamepad: intros, Continue, walk, camera
  turn, Witcher Senses, game menu, then EXIT through the system menu (the game closes itself,
  the `quit` fallback is not needed).
- Virtual gamepad for 5.01 test runs: the game registers one GameInput device callback for
  gamepads and then queries `GetCurrentReading(Gamepad, device)` for each reported device. With
  a script loaded and no real gamepad reported 3 s after the first Present, the test mode reports
  a virtual device through that callback and answers its queries with a reading that carries the
  script pad. The game only calls `GetDeviceInfo` on it. Verified: the full smoke script runs
  with the gamepad switched off and disconnected.

## 2026-10-12

### Smoke test
- `scripts/smoke-test.ps1` runs the 5.01 smoke script unattended (no controller needed) and
  checks the log: script loaded, all captures written, no hook failure or signature mismatch,
  no timeout. A full run takes about 90 s.

### First engine hooks on 5.01 (per-build address table)
- `src/engine_addresses.h` holds per-build RVAs. 4.04 keeps its original values. A 5.01 entry
  also carries the expected function bytes, which are checked at runtime before hooking
  (`engine_function_target()`); a mismatch disables that hook. Hooks resolved this way run on
  5.01 although `engine_build_supported()` is still false for it.
- Method: the 5.01 address comes from the earlier exe matching; the 4.04 function bytes must
  occur exactly once in the 5.01 exe, at that address. Struct offsets the hook (or code
  mirroring it) relies on must be visible and unchanged in those bytes.
- `IsAnyMenu` (5.01 RVA 0x01DF41A0, whole 35-byte function identical): verified on 5.01, the menu
  state toggles while loading and with the game menu (START/B).
- `IsLoadingScreenVideoPlaying` (5.01 RVA 0x0227A0F0, identical incl. the +0x198/+0x38/vtable
  +0x18 layout mirrored by `poll_engine_loading_screen_video_state()`): moved out of the
  camera-authority installer into its own installer.
- Scene queries, unique and byte-identical in 5.01, now resolved through the table:
  `Actor.IsInGameplayScene` 0x0210B410 (+0x2F3), `Actor.IsInNonGameplayCutscene` 0x0210B430
  (+0x2F2), `Game.IsCurrentlyPlayingNonGameplayScene` 0x022753C0 (+0x11F). The flags read by
  `read_native_camera_authority_snapshot()` are the same offsets. Verified on 5.01: all three
  hook; the game calls the cutscene and global scene queries (result 0 in free roaming).
  `IsInGameplayScene` was not called on this route (on 4.04 mainly the mod scripts call it,
  which are not installed in the 5.01 test setup).
- **5.01 is an AVX build** (VEX-encoded SSE instructions), so most functions are not
  byte-identical even where the logic is unchanged; byte comparison only works for small
  integer-only functions. **Camera object layout moved:** `GetCameraDirection` reads the
  direction at +0x790 in 5.01 (4.04: +0x6C0, so +0xD0). The camera director list layout
  (+0x58 entries, +0x60 count, stride 0x28) is unchanged. Every struct offset used on camera
  objects therefore needs a 5.01 check.
- `CameraDirector.GetCameraDirection` located at 5.01 RVA 0x023826F0 (registered script name;
  both builds have an identical second copy 0x110 bytes later, the hooked one is the first in
  both). The hook reads no camera fields, but **MinHook cannot hook it on 5.01**:
  `MH_CreateHook` returns `MH_ERROR_UNSUPPORTED_FUNCTION`, because its disassembler (HDE64)
  rejects VEX-prefixed instructions and the function has one at byte 4, inside the 5 bytes the
  jump overwrites. This affects small leaf functions; the larger 5.01 candidates (frame data
  factory, view rebuild, temporal writer, gameplay entry, head-bone getters, ...) start with
  plain prologues. Options: patch the native-function registration instead of the code for
  script natives, or teach the trampoline builder VEX instruction lengths. Left 4.04-only.
- Still 4.04-only in the camera-authority installer: `GetTopmostCameraObject` (5.01 candidate
  0x023825D0, compiled differently but same director layout: +0x58 entries, +0x60 count,
  stride 0x28) and `EnableManualControl` (5.01 RVA 0x01E0D800 by registered script name, plain
  prologue). Comparing `EnableManualControl` shows the **custom-camera fields moved by +0x10**
  in 5.01: manual-control flag 0x259 → 0x269, float pairs 0x248/0x250 → 0x258/0x260; the +0x1C0
  pointer is unchanged. The hook and `read_native_camera_authority_snapshot()` read 0x259, so
  enabling them needs a per-build field offset. Worth doing together with the view/frame
  hooks that consume the snapshot.
- `GameplayEntry` (4.04 0x01D00260) confirmed at 5.01 RVA 0x01D0C570: instruction-for-instruction
  the same structure. Fields of the global game object it reads moved by +8 (0x78 → 0x80,
  0x80 → 0x88, 0x88 → 0x90) and +0xC (0x91 → 0x9D); fields of the inner object (0x280, 0x2A0,
  0x70) are unchanged. Not hooked yet: it only matters together with the frame builder and its
  return-address identities.
- 4.04 regression (commit `885c229`, flat mono smoke test): passed. All table-resolved hooks
  land on the original 4.04 addresses; the game calls all three scene queries there.

### Third-person stereo core on 5.01 (mapping)
The second eye comes from the frame-factory hook (calls the factory twice, enqueues the second
frame itself); the view-rebuild hook sets the per-eye camera; gameplay entry maps frames to eyes on
the render thread; render-scene setup paces pairs. Located through the call graph of both builds
(Ghidra decompilation, local only) and the `CRenderCommand_RenderScene` / `CCustomCamera` RTTI:

| Role | 4.04 | 5.01 | Evidence / layout notes |
|---|---|---|---|
| Render-scene setup | 0x01553070 | 0x0227D1F0 | calls the factory, builds a RenderScene command |
| Frame-data factory | 0x016214C0 | 0x022C0820 | calls view rebuild, then the same wrapper |
| Scene enqueue | 0x01621FC0 | 0x022BE990 | same logic; render-context field +0x620 → +0x5440, scheduler global 0x057F5930 → 0x05A5B8F0 |
| View rebuild | 0x015FE550 | 0x02293240 | called from the factory |
| Factory → view rebuild return | 0x0162196D | 0x022C101A | |
| Wrapper → view rebuild return | 0x015FF863 | 0x02292EA7 | |
| Gameplay entry → frame builder | 0x01D00260 → 0x01D86400 (return 0x01D002CF) | 0x01D0C570 → 0x01C1A210 (return 0x01D0C5E1) | task +0x8 frame, +0x10 extra unchanged |
| `CCustomCamera` vtable (subobject +0x170) | 0x02D645B0 | 0x03810198 | RTTI complete-object locators |

Layout changes that block a direct port:
- Scene descriptor: the factory reads up to about +0xB765 in 4.04 (the mod copies 0xC000) and up
  to about +0xF714 in 5.01. The second-eye copy size must grow accordingly (to be confirmed).
- View array: indices 0x00–0xAB (position, angles, FOV, aspect, near/far, projection at 0x80,
  basis at 0xA0/0xA4/0xA8) are used identically. **Centre offset and viewport moved from
  view[0x100..0x103] to view[0x130..0x133]**; 5.01 uses 0x100–0x12F for matrix copies, so the
  hook's writes to view[0x100]/[0x102] must not run unchanged on 5.01. view[0x11A] has a new
  meaning as well.
- Ported through the address table (5.01 values in `src/engine_addresses.h`): frame factory,
  scene enqueue, view rebuild, gameplay entry, render-scene setup, `GetTopmostCameraObject`,
  the CCustomCamera vtable, both view-rebuild return addresses, the scene-descriptor copy size
  (5.01: 0x10000, minimum 0xF800) and the centre-offset/viewport index (`engine_view_centre()`,
  0x100 on 4.04, 0x130 on 5.01; all 30 accesses go through it). Frame builder, temporal writer,
  shadow cascade and the TAAU/DLSS hooks stay 4.04-only.
- Verified on 5.01 (flat, `mode=3`, OpenXR off): all of these hook; at runtime the director's top
  camera carries exactly the predicted 5.01 CCustomCamera vtable (0x03810198) and the scene flags
  read 0 in free roaming; the view rebuild accepts the gameplay camera and renders the eyes
  alternately per frame (two consecutive captures show the expected horizontal offset), as the
  4.04 flat stereo run does. The real dual render (two frames per Present) only starts with a
  valid HMD pose, so it needs an OpenXR run with the headset.
- 4.04 regression of this build (commit `328679e`, flat stereo `mode=3`): passed; the alternating
  eyes show the same offset as before and stay sharp (the mod's per-eye TAAU history).
- On 5.01 the alternating eyes blur: the game's own temporal anti-aliasing blends left and right
  frames, and the per-eye history only exists for 4.04 TAAU. The 5.01 VR baseline should use FXAA
  (no temporal history); the strict pair mode renders both eyes per Present, but consecutive
  frames still alternate eyes, so temporal AA would mix them there as well.
- Not ported yet: frame builder and temporal writer, which 4.04 needs for the asymmetric
  projection (`native_stereo=1`). On 5.01 the engine itself applies the centre offset at
  view[0x130] when building the projection, so a first headset test should start with symmetric
  projection (`native_stereo=0`) and then compare.
- Temporal writer located at 5.01 RVA 0x022BD750: same logic as 4.04 0x015E5A90 (writes the
  centre/jitter block of two views, calling the view rebuild after each). It shows the 5.01 frame
  layout: first view at frame+0x10 in both builds, second view at +0x5F0 instead of +0x520 (view
  stride 0x510 → 0x5E0), centre blocks at +0x4D0/+0xAB0 instead of +0x410/+0x920. 5.01 calls it
  from the frame builder once and from two other functions; render-scene setup no longer calls it,
  so the 4.04 return-address list does not carry over. Frame-builder offsets (+0x420/+0x930,
  +0x428/+0x938, +0x530, +0xA4C, +0x560) need to be re-derived with this stride.

## 2026-10-13

### Headset-free OpenXR runs (SteamVR null driver)
- The SteamVR runtime with its built-in null driver provides a virtual HMD (fixed pose, 72 Hz,
  configurable render size) with the full OpenXR frame loop: session, swapchains, xrWaitFrame,
  xrEndFrame. `scripts/smoke-test.ps1 -OpenXrRuntimeJson <SteamVR>\steamxr_win64.json` selects it
  for the game process only (`XR_RUNTIME_JSON`); the system runtime stays unchanged. `-IniSet`
  changes INI values for one run (`openxr.enabled=1`, `openxr.native_stereo=0`, ...).
- SteamVR needs `steamvr.forcedDriver=null`, `steamvr.requireHmd=true`,
  `steamvr.activateMultipleDrivers=false`, `driver_null.enable=true` and
  `steamvr.startMonitorFromAppLaunch=false`. Without the last one vrmonitor launches Room Setup
  for the uncalibrated null HMD, which sends the game a quit event.
- The null driver reports a constant orientation with 180° roll; use
  `openxr.hmd_roll_scale=0` in these runs, otherwise the scene renders upside down.
- Nothing about the headset path is covered (lenses, real tracking, Pico/VDXR specifics), but the
  complete mod pipeline runs: dual render, pair barrier, OpenXR submission, HUD/smoke routes.

### First dual render on 5.01 (Mode 3, symmetric projection)
- Verified unattended (smoke script, null HMD, `native_stereo=0`): the dual render starts, both
  eyes are produced per pair through the 5.01 frame factory / scene enqueue / gameplay entry,
  captures of consecutive presents show the expected horizontal eye offset, and the run ends
  normally. Scene-descriptor copy on 5.01: 0xFD20 bytes readable (the factory reads up to about
  +0xF714), so the 0x10000 copy size with zero-padded tail holds.
- Initially only ~25 presents/s with ~100 timed-out pairs per run. The new render-thread stack
  sampler (`[debug] thread_stack_sampler=1`, report through the test-mode `flight_dump` action)
  showed ~60% of the render thread inside DXC: the real-smoke world-up GS was recompiled on
  every call because its PSO creation kept failing. With a retry limit: ~90 presents/s.
- Cause of the PSO failure: the 5.01 root signature binds VS b0-b4 as VERTEX-only root CBVs and
  PS b0-b4 as PIXEL-only root CBVs, has no GS parameters and sets
  `DENY_GEOMETRY_SHADER_ROOT_ACCESS` (flags 0x1D). 4.04 has per-stage CBV tables including one
  for the GS (the smoke route copies VS table 3 into GS table 6). On 5.01 the mod now appends a
  GEOMETRY root CBV b1 to root signatures with that layout, drops the GS deny flag and binds the
  appended parameter to the VS b1 address at the smoke draw. The root signatures are created
  before the swapchain on 5.01, so the CreateRootSignature hook is installed from a
  D3D12CreateDevice hook on that build. Result: all three smoke projection PSOs are created,
  ~6 timed-out pairs per run (startup), Present pacing ~11 ms.
- Same as 4.04 (no port issue): the cinema detector's alternating "gameplay authority
  lost/confirmed" lines and the dual-render auto-start reason `automatic_full_vr`.
- Open on 5.01: the foliage native snapshot (V1521) falls back on every draw
  (`reason=graphics_state`): it expects the 4.04 VS CBV table with b0 and b12; on 5.01 b0 is a
  root CBV (parameter 5) and b12 sits in the b5-b15 table (parameter 17). Needs its own 5.01
  binding path.
- Not yet run with these changes: 4.04 regression (the 4.04 paths are unchanged by design:
  early hook and root-signature extension are 5.01-only, the smoke retry limit only matters when
  creation fails).
