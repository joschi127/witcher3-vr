# Remaster port: status log

Chronological record of what has been verified. Details live in `remaster-porting-analysis.md` (code
dependencies, runtime findings) and `remaster-research.md` (public information). Raw logs,
reverse-engineering output and machine-specific data stay out of the repository.
Current known problems and untested areas: `remaster-open-issues.md`.

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
  in `docs/guidelines.md`).

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
  Successful `xrEndFrame` calls alone do not prove that images were submitted: check the
  `OpenXR layer transition` lines (and the null driver's "Headset Window"). The null HMD also
  enters standby after a few seconds without motion (session drops to SYNCHRONIZED); set
  `power.pauseCompositorOnStandby=false` and a long `power.turnOffScreensTimeout`. SteamVR reports
  every null-HMD frame as dropped on both builds; that counter is meaningless there.

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

### Further 5.01 ports (verified with the null HMD unless noted)
- Frame builder (5.01 0x01C1A210, called from the gameplay entry, return 0x01D0C5E1) and the frame
  offsets its helpers read, all in the address table: secondary view core +0x600, centre/extent
  blocks +0x4E0/+0xAC0 (+8), input extent +0xBEC, camera transform +0x630. The builder reads the
  input extent at +0xBEC (4.04 +0xA4C) and passes frame+0x10 to the temporal writer, which writes
  +0x4D0/+0xAB0 relative to it. Enables the cinema/Full VR final-frame camera; not exercised by the
  smoke route (no cutscene). The native asymmetric route stays 4.04-only (temporal writer hook).
- `CCustomCamera.EnableManualControl` (5.01 0x01E0D800): the setter stores the flag at +0x269
  (4.04 +0x259) and copies the float pairs at +0x258/+0x260 (4.04 +0x248/+0x250). The camera
  authority snapshot reads the director's top camera with the same table offset, but that pointer
  carries the +0x170 subobject vtable, so its `top_manual` value is diagnostic only.
- `GetCameraDirection` (5.01 0x023826F0) through `src/prefix_hook.h`: relocates the verified 20-byte
  prefix (inc [rdx+0x30]; vmovss x2) that MinHook cannot decode. Called in gameplay, returns a unit
  direction.
- Viewport resolution (5.01 0x0228F3B0, same signature and logic; the DXGI desktop-size part moved
  into 0x0228E230): `render_width/height` now set the game's render size on 5.01.
- HUD composite: runtime pair VS 2668F02C9B6C7FDD / PS 0816B1C02F3BAF8B (the offline matcher had
  picked another VS). The 5.01 PS adds a colour-blindness filter on the scene colour
  (CustomPixelConsts row 17: mode .x, strength .w, active when .w > 0), otherwise it is the 4.04
  composite the replacement HLSL mirrors. Verified: per-eye HUD variants are created and the HUD
  shifts by ±36 px between the eyes.
- 5.01 anti-aliasing options (`user_config_matrix/pc/graphics.xml`): `AAMode` 0 off, 1 FXAA,
  5 FSR, 6 DLSS, 7 XeSS. VR baseline on 5.01: FXAA.
- The HUD frame scale is written to `dx12user.settings` only inside `witcher3.exe` (ctest no
  longer touches the user's settings).
- VDXR with the user's Virtual Desktop settings reports symmetric eye FOVs (centres 0.5/0.5), so the
  first headset run can use `native_stereo=0` without losing the optical centre.
- 4.04 regression of these changes (commit `33ae4d2`): flat stereo smoke run passed with the same
  hook set as the previous regression run; a null-HMD OpenXR run with the normal 4.04 VR
  configuration (TAAU, `native_stereo=1`, 1748x1468) passed as well: dual render, smoke projection
  PSOs and per-eye HUD PSOs created, 3 timed-out pairs, Present pacing ~11 ms. The 5.01-only
  paths (early CreateRootSignature hook, root-signature extension) stay inactive on 4.04.

### First headset run on 5.01 (Pico 4, Virtual Desktop) and fixes
- Run 1 (`33ae4d2`): session FOCUSED, but the headset stayed black. At startup 5.01 briefly
  presents an R10G10B10A2 backbuffer; the menu/panel projection path copied it into the RGBA8
  swapchain, the D3D12 debug layer rejects that copy, the XR command list failed to close and was
  never usable again, so no later frame carried a layer. The earlier null-HMD runs had the same
  failure (layer `none` after present 3). Fixed in `dd02d89` (format-family guard on the direct
  copy, recreate the list after a failed close; `[debug] d3d12_debug_layer=1` logs the debug
  layer's errors).
- Then both headset eyes flickered in gameplay (steady in menus and on the map), confirmed in the
  null-HMD headset view on 5.01 and absent on 4.04. Cause: `temporal_backend=taau` made the packed
  pair promotion wait for TAAU submissions that 5.01 never produces, so both eyes were fed from the
  alternating backbuffer. Fixed in `4ad8fbd` (5.01 forces the No-AA route). Null-HMD view: steady,
  one image per eye.
- The menu panel appears upside down in the null-HMD view only (180° roll of the null driver).
- Run 2 (`4ad8fbd`, Pico 4 / Virtual Desktop): image in both eyes, intro, menus and loading
  screens as on 4.04, gameplay smooth, but no usable depth; the user suspected swapped eyes.
  Confirmed without the headset: the test mode now also saves the two images submitted to OpenXR
  (`xr_eye0/1` captures). On 4.04 near objects shift left in the right-eye image; on 5.01 they
  shifted right. The built frames had the correct cameras (right eye +IPD along the right
  vector), so the swap happened when the presented backbuffer was tagged with its eye: 5.01
  records the backbuffer PRESENT barrier on the render worker inside the gameplay frame task,
  before that task's completion tag is queued, so the capture took the previous task's tag
  (the other eye). 4.04 records it afterwards on another thread. Fixed in `c892871`: a PRESENT
  inside the running frame task uses that task's own tag. Null-HMD captures now show the 4.04
  parallax direction. Headset check pending.

### First playable VR on 5.01 (headset run 3, `c892871`)
- Pico 4 over Virtual Desktop (VDXR), symmetric projection (`native_stereo=0`), 1748x1468 per eye,
  FXAA, No-AA route. Third-person gameplay in Kaer Morhen, indoors and on the balcony.
- Result (user): real 3D depth, correct in both eyes (checked with one eye closed at a time),
  intro, menus, loading screens and menu panel as on 4.04, performance subjectively close to 4.04,
  fire/smoke, HUD and trees look right. Log: every gameplay projection submits the packed pair
  (one image per eye), no timed-out pairs, Present pacing ~7.4 ms.
- Open:
  - Distant water (river seen from the balcony) slowly alternates between dark and light (about
    once per second, also with camera movement), in both eyes at the same time as far as the user
    can tell. So probably not an eye-alternation artefact; candidates are a reflection/water shader
    or exposure effect, or a mod hook acting on it on 5.01. Not yet checked flat (without the mod)
    or on 4.04 at the same spot.
  - Colour saturation in VR felt lower than on 4.04 (subjective). The mod passes colours through
    unchanged on both builds (monitor and submitted eye image of the same frame have equal mean
    saturation). 2026-10-14, balcony save made on 4.04 (5.01 loads it via Continue), null-HMD and
    flat captures of the same spot, mean saturation / value: 5.01 flat mono 0.244 / 0.395, 5.01 flat
    stereo (alternating eyes) 0.243 / 0.395, **5.01 OpenXR 0.150 / 0.525 (hazy, washed out)**, 4.04
    OpenXR 0.213 / 0.448. So the haze is specific to the 5.01 OpenXR path, not the game itself and
    not the eye alternation. Narrowed down further (same spot, mean saturation / value): flat with
    the forced 1748x1468 render size 0.247 / 0.407 (vivid), OpenXR without HMD camera
    (`hmd_freelook=0`) 0.148 / 0.482, OpenXR mono without dual render 0.154 / 0.494, OpenXR with the
    game's own projection (`hmd_compositor_only=1`, view parameters identical to flat) 0.157 / 0.462.
    So render size, HMD camera, dual render, eye alternation and projection are ruled out, and so is
    the mod's replacement HUD composite (VR with `[debug] disable_hud_composite_replacement=1`:
    0.152 / 0.491). Indoors there is no difference (flat 0.742 vs VR 0.732). **Cause: the 5.01 light
    shafts ("God rays", `[PostProcess] AllowShafts`, on by default in both builds).** With the sun in
    view 5.01 lays a bright veil over the whole image, also flat without OpenXR (camera tilted
    towards the sun: 0.186 / 0.509 vs 0.273 / 0.351 looking down). The wider VR field of view keeps
    the sun or a large part of the sky in view, so the veil is almost always there. With
    `AllowShafts=false`: flat towards the sun 0.266 / 0.443, OpenXR balcony 0.269 / 0.465 (vivid,
    more saturated than 4.04 OpenXR at 0.213). Not a mod defect; the 5.01 VR settings profile
    switches light shafts off (to be applied by the build-aware launcher later). The 5.01
    settings add `[PostProcess] WaterFogPerPixel=true` (also of interest for the water flicker). The water flicker does not occur on 4.04 at the same spot.
  - Aiming (crossbow) untested; cinema camera and cutscenes not yet checked in the headset.
  - Not ported: foliage native snapshot (V1521), native asymmetric projection, TAAU/DLSS/RT
    backends.

### Rough VR performance comparison (2026-10-14)
- SteamVR null driver at 240 Hz (so `xrWaitFrame` does not cap), same balcony save on both builds,
  15 s standing plus 8 s slow camera turn, no captures, identical diagnostic settings, AMD RX 7900
  XTX class GPU. One run each, so only the direction is meaningful.
- Mean time per Present in gameplay: 5.01 (FXAA, No-AA route) ~8.4 ms at 1748x1468 and ~8.8 ms
  at 2448x2448 per eye; 4.04 ~15 ms in all three variants (TAAU at both sizes, and FXAA with
  `temporal_backend=none` at 1748x1468). SteamVR app CPU/GPU time: 5.01 ~10.5/4 ms, 4.04
  ~11.7/3.3 ms.
- 5.01 is not slower in VR; on this machine it delivers about 1.8x the frame rate. Both builds
  are CPU-bound here: 2.3x the pixels per eye barely changes the frame time, so a GPU-limited
  comparison (weaker GPU) is still open.
- The mod's diagnostics are expensive: the same 5.01 run (1748x1468) with all `[debug]` switches
  off (`logging_enabled`, `runtime_diagnostics`, both flight recorders, the TAAU-drop, cinema,
  first-person and world-marker diagnostics) cuts SteamVR's app CPU time from 10.4 ms to 6.0 ms
  per frame and GPU time from 3.9 ms to 2.5 ms (about 40 % less CPU). The numbers above were all
  measured with diagnostics on, so they overstate the cost for normal play. Turn diagnostics off
  for playing and for performance judgements; without them the mod writes no log, so smoke
  tests need them on (`-IniSet 'debug.logging_enabled=1,debug.runtime_diagnostics=1'`).

## 2026-10-15

### Headset play session at 90 Hz (diagnostics off)
- Pico 4 over Virtual Desktop at 90 Hz, same FOV and render size as before, all `[debug]`
  diagnostics off (no log).
- Cutscenes work in the headset, including the cinema camera position.
- Combat (combat tutorial, first ghoul fight) works well. Horse and crossbow not yet tested.
- New findings (details in `remaster-open-issues.md`): background trees in the first Ciri cutscene
  follow the head; the distant river switches between blue and white with the head pose (also per
  eye, light shafts off); one kind of grass is always almost black (courtyard and elsewhere); the
  background-tree problem also appeared in a second cutscene. Overall: nice and playable at 90 Hz.

## 2026-10-16/17

### Black grass: AMD driver, not the port
- One kind of grass rendered solid black everywhere outdoors. Reproduced without a headset with an
  outdoor save, both with OpenXR (null driver) and flat without OpenXR. Not changed by any of: ray
  tracing on, the texture keys copied from the 4.04 profile removed, grass/SpeedTree/foliage values
  at their 5.x defaults, FSR instead of FXAA, the game's native 2560x1440 without the forced render
  size (where the mod no longer changes rendering).
- Community reports describe the same black grass on Radeon GPUs since the 5.0 update. The game's
  launcher also asks for a newer driver. The automatic AMD update offered only Adrenalin 26.8.1;
  26.9.2 WHQL (full download from the AMD website) fixes it: grass correct flat and in the OpenXR
  eye images with the same save and script.
- AMD users need Adrenalin 26.9.2 or newer for 5.x.

### Test action for head motion
- New script action `hmd <yaw> <pitch>` turns the located OpenXR head pose, so head-motion bugs can
  be reproduced with the static null HMD (verified: 30 degrees yaw and -25 degrees pitch turn the
  submitted eye images as expected).

### Cutscene background following the head: fixed (`8d94088`)
- In some cutscenes trees and sky stayed fixed in the image while the head turned (reported from
  the headset on 2026-10-15). Reproduced without a headset with a 5.01 checkpoint before an evening
  cutscene and image pairs at 0 and 20 degrees synthetic yaw: characters and ground shifted, trees
  and sky did not move by a pixel.
- Cause: these cutscenes reuse the native camera stored in the frame data, so the mod's Full VR
  fallback (`prepare_full_vr_frame_camera`) writes the HMD-corrected camera into the frame
  itself. It corrected only the primary view at frame+0x20; the frame carries a second camera
  copy right after it (`FrameSecondaryViewOffset`, 0x600 on 5.01, 0x530 on 4.04), and sky and
  trees read that one.
- Fix: mirror the corrected camera into the secondary view for the frame build and restore it
  afterwards, like the primary. Verified with the null HMD (whole image turns together) and in
  the headset (Pico 4, Virtual Desktop): background no longer attached to the head.
- The code path is the same on 4.04, which likely had the same problem (upstream lists "some
  terrain can still look incorrect in a few Full VR cutscenes" as a known issue). Open: a 4.04
  regression run through such a cutscene.
- Found in the same scene: a cutscene camera light that follows the head (see open issues).
