# CLAUDE.md

Fork of [tig3rmast3r/witcher3-vr](https://github.com/tig3rmast3r/witcher3-vr) to port the mod
from The Witcher 3 Next-Gen (DX12, patch 4.04) to the newer "Remaster" build (GOG edition
`remasteredEdition`, `witcher3.exe` 5.0.0.x). Hobby project. Upstream README/CONTRIBUTING
remain authoritative for everything not port-specific.

Machine-specific paths and status live in `CLAUDE.local.md` (git-ignored). Read it if present.

## Build

Run from the **Developer PowerShell for VS** (cmake/cl are not on the plain PATH). The presets
use the "Visual Studio 17 2022" generator. With only VS 2026 installed, add a git-ignored
`CMakeUserPresets.json` preset that inherits `release` with generator "Visual Studio 18 2026"
and its own `binaryDir`. Upstream builds cleanly that way (MSVC 14.51, all tests pass).

```powershell
.\scripts\setup-dependencies.ps1   # sparse-clones pinned deps into external/ (network)
cmake --preset release             # VS 17 2022, x64, build/release
cmake --build --preset release
ctest --preset release
```

Output is `dxgi.dll` (proxy, `src/dxgi_proxy.def`) plus `Witcher3VRLauncher.exe`.
`scripts/package-release.ps1` shows the install layout (`bin/x64_dx12`, `mods/`, `dlc/`,
`bin/config/r4game/user_config_matrix/pc`).

A full package cannot be built from this repo alone. `package-release.ps1` requires
prebuilt third-party binaries under `runtime/` and `external/` that are not in git:
PureDark AFW plugin, OFXR API layer, the ReShade and OptiScaler forks, RenderDoc.

Dev install procedure:
1. Install the official release once into the target game.
2. Replace only the self-built files: `dxgi.dll` goes to
   `bin\x64_dx12\witcher3vr-mod-reference\dxgi.dll`, and the launcher goes next to it in
   `bin\x64_dx12`. The launcher creates the active root copies from the reference folders,
   so a `dxgi.dll` placed directly in `bin\x64_dx12` may be overwritten.
3. Replace changed `support/` scripts in `mods\` as needed.

## Tests: what they do and do not cover

- `tests/*.cpp`: unit tests of header-only policy modules (eye geometry, AER scheduling,
  HUD/transport policies, etc.). No game involved.
- `tests/*.cmake` "contract" tests: `file(READ)` + `string(FIND)` over source text. They assert
  that code fragments exist or were retired. They check text, not behaviour. Editing an
  asserted fragment breaks them, which may be intended. Update them deliberately.
- `launcher/tests.cpp` etc.: settings/path logic against a fake layout.
- **`ctest` has a side effect on the real game settings.** `swapchain_identity_tests` calls
  `CreateDXGIFactory1` from the build output directory, so Windows loads the freshly built
  proxy `dxgi.dll` next to it. Its `load_config()` → `apply_hud_frame_scale()` writes
  `[Hidden] uiHorizontalFrameScale/uiVerticalFrameScale` into
  `%USERPROFILE%\Documents\The Witcher 3\dx12user.settings`, creating the file if it is missing.
  Back up that file before running tests, or fix the test isolation (candidate upstream issue).
- **Nothing tests compatibility with a real `witcher3.exe`.** A green `ctest` says nothing
  about whether hooks, offsets or shader hashes match a game build. That needs the smoke test
  (planned: `scripts/smoke-test.ps1`: build, start game with diagnostic logging, ~60 s,
  quit, evaluate `witcher3vr.log`, exit code) and finally a headset run by the user.

## Launcher behaviour worth knowing

"Configure Settings for VR" (`ConfigureGameSettingsForVr`, `launcher/config.cpp`) replaces
`Documents\The Witcher 3\dx12user.settings` with the bundled `launcher/vr_dx12user.settings`.
Only the `[Galaxy]` and `[Localization]` sections and the camera inversion keys in `[Input]`
(`InvertCameraX/Y`, `...OnMouse`) are preserved. Other user preferences are reset. A one-time original backup is created and is restored by
"Restore Original Settings".

## Architecture (short)

Almost everything is in `src/dxgi_proxy.cpp` (~54k lines). Entry: `DllMain` → `ensure_initialized`
(loads `witcher3vr.ini`, MinHook, then `install_engine_*` hooks). D3D12/DXGI vtable hooks track
resources/PSOs (FNV-1a hash of shader bytecode), game-internal hooks at fixed RVAs drive the
camera/stereo logic, OpenXR session + D3D12 swapchains submit in Present. Modes: Mono, Stereo
(engine frame factory called twice per Present), AER+AFW (alternate eye + PureDark reprojection).
Witcher scripts in `support/mod*` talk to the DLL through hooked native script functions.
Details and the full list of build-dependent code: `docs/porting-analysis.md`.

## Porting rules

- **Never run unmodified upstream builds on the Remaster.** Upstream has no version check.
  Its game hooks are raw RVAs patched blindly, and six are installed unconditionally at
  startup. This fork's build guard (`src/game_build.h`) disables engine hooks on unknown or
  unsupported builds. Only builds that contain it may be used on 5.01. Progress log:
  `docs/port-status.md`.
- Keep the two game installations strictly separate. Never copy mod files into the wrong one.
  Both share `Documents\The Witcher 3` (saves, `dx12user.settings`), so back it up before
  switching versions or using the launcher's "Configure Settings for VR".
- Prefer changes that keep 4.04 working (per-build address tables / pattern scans) until the
  "one or two builds" decision below is made.
- Ask the user before side effects: installs, pushes, deletes, anything in game directories
  or `Documents\The Witcher 3`.
- No credentials or tokens.

## Data hygiene (what may be committed)

Allowed in git: source, tests, and docs whose content is derivable from the public source
(constant names, hook lists, methodology, plans).

Never commit (keep under `local/` or in `CLAUDE.local.md`, both git-ignored):
- game files/assets, game or NVIDIA DLLs, save files
- user settings files from `Documents\The Witcher 3`: `dx12user.settings` contains an account
  refresh token (`[Galaxy] tokenRefA`)
- runtime logs (`witcher3vr.log`, OpenXR API dumps, crash dumps), screenshots with HUD/game content
- reverse-engineering artifacts: Ghidra/IDA projects, decompiled code, byte dumps, exe hashes
- machine/user data: local paths, user names, exact hardware identification (driver versions,
  serial numbers, device IDs), Claude session transcripts

The test setup may be named at the level needed to interpret a result: GPU vendor or model,
headset model, OpenXR runtime, driver module names (e.g. "AMD GPU, Pico 4 over Virtual Desktop").

Address constants and byte patterns needed by the code are fine in source (upstream does the
same). The material they were derived from is not.

## Git workflow

- Remotes: `origin` = the fork (https://github.com/joschi127/witcher3-vr), default push target.
  `upstream` = tig3rmast3r/witcher3-vr, fetch only. Never push to `upstream`.
- `main` mirrors `upstream/main` and gets fast-forwards only, no fork commits.
- `remaster` is the working branch for the port. Commit directly to it in small, self-contained
  commits (easy to cherry-pick for upstream PRs later). Every commit must build.
- Which commit was verified in the game (5.01, plus a 4.04 regression check where relevant) is
  recorded in `docs/port-status.md`, including open checks. That is the reference for "known good".
- Separate branches only for experiments that will likely be thrown away.
- **No `Co-Authored-By: Claude` (or other AI attribution) trailers in commit messages or PR
  descriptions.** Commits are authored by the repository owner only. The point is not to hide
  the use of Claude. GitHub would list Claude as a contributor otherwise.
- Check before every push: no files from the data-hygiene "never commit" list, no binaries,
  no local paths or exact hardware identification in the outgoing commits.

## Port roadmap (agreed with the user)

- **Build guard** (`src/game_build.h`, `engine_build_supported()` in `dxgi_proxy.cpp`): engine
  hooks run only on recognized, supported builds. Next: a per-build address table for the
  Remaster, enabled hook by hook with log verification.
- **Launcher "Configure Settings for VR" must become build-aware.** Whatever is changed by hand
  from the game defaults during the port must later be applied automatically by that button,
  per detected build. This includes settings switched off to stay close to 4.04 (RT/path
  tracing, ray reconstruction, frame generation, Anti-Lag/low latency, HDR, editor preset
  detection) and AA/AO values whose meaning changed in 5.x (`AAMode`, `SSAOEnabled`). The
  current 4.04 profile (`launcher/vr_dx12user.settings`) must never be applied to 5.x. Keep
  the list of manual deviations up to date until the launcher implements it.

## Open decisions

- One fork supporting both 4.04 and Remaster, or a separate Remaster-only fork? Decide after
  checking whether shader hashes and engine struct layouts changed (see analysis doc).

## Upstream relationship

- Upstream currently supports only Next-Gen 4.04 (latest Next-Gen update), not the Remaster.
  CONTRIBUTING's "latest official PC version only" rule predates the Remaster and should not
  be read as Remaster support.
- Remaster support was asked about upstream in
  [issue #27](https://github.com/tig3rmast3r/witcher3-vr/issues/27). Check it for news before
  larger work, to avoid duplicating an upstream port.
- As of 2026-10-07 the author intends to port the mod to the Remaster himself, "not this
  month" ([#31](https://github.com/tig3rmast3r/witcher3-vr/issues/31)). The Next-Gen version is
  frozen upstream. Findings from this fork may be useful to him. See
  `docs/remaster-research.md` for the public information collected so far.
- Upstream's scope rules apply to bug reports: do not report problems from this fork or from
  Remaster builds upstream as regular bugs.
- Once this port shows real progress, the user (author of issue #27) will contact the author to
  coordinate. Options include joint work on a fork, independent forks, or upstream PRs if
  one codebase can support both builds. That decision belongs to the user. Do not contact
  upstream on the user's behalf. Keep changes focused and upstream-friendly: small
  commits, English, upstream code style, no unrelated refactors.
