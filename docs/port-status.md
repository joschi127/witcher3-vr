# Remaster port: status log

Chronological record of what has been verified. Details live in `porting-analysis.md` (code
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
See `porting-analysis.md` §6. In short:
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
- Open: 4.04 regression run with this build, then compare the 28 shader fingerprints on 5.01.
  The first recording of a newly created list (before its first Reset/submission) is not yet
  intercepted. Hook `CreateCommandList`/`CreateCommandList1` if that turns out to matter.
