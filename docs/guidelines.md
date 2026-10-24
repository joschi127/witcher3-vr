# Project guidelines

Guidelines for everyone working on this fork: developers and AI coding agents alike. Agent
entry files (`CLAUDE.md`, `AGENTS.md`, `.junie/guidelines.md`,
`.github/copilot-instructions.md`) only point here; keep the content in this file.

Fork of [tig3rmast3r/witcher3-vr](https://github.com/tig3rmast3r/witcher3-vr) to port the mod
from The Witcher 3 Next-Gen (DX12, patch 4.04) to the newer "Remaster" build (GOG edition
`remasteredEdition`, `witcher3.exe` 5.0.0.x). Hobby project. Upstream README/CONTRIBUTING
remain authoritative for everything not port-specific.

"The maintainer" below is the repository owner, who decides on side effects, pushes and
upstream contact.

## Local notes (git-ignored)

Machine-specific paths, the current state and the to-do list live outside git, under `local/`
(git-ignored as a whole). Read them first if present; they are not part of the repository.

- `local/NOTES.md`: current state only. Environment, game installations, installed builds,
  test commands, open points and the to-do list for the next session. Keep it short and up
  to date; replace outdated statements instead of appending.
- `local/HISTORY.md`: older status snapshots and completed steps, in chronological order. When
  a section of `NOTES.md` is no longer current, move it here (dated) rather than deleting it.
- `local/README.md`: what else is stored under `local/` (runs, backups, tools, settings copies).
- `CLAUDE.local.md` (repo root, git-ignored) only imports `local/NOTES.md` for Claude Code;
  other tools and developers read `local/NOTES.md` directly.

The notes may contain local paths and machine details; that is why they stay out of git. Their
language is the maintainer's choice; committed files are in English.

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

### Releases of this fork

`scripts/package-fork-release.ps1 -Version <name> -ReleaseNotes docs/releases/<name>.md` builds the
release zip from the pinned original release (downloaded with `gh`, SHA256-checked; third-party
runtime files and game scripts unchanged) plus this repository's `dxgi.dll`, launcher, documents and
example INI. It leaves out PureDark's `PDAFWPlugin.dll` and the Fast Transitions DLC (not
redistributed, see `THIRD_PARTY_NOTICES.md`). Output goes to `local/releases/`.

Procedure when the maintainer asks for a release:
1. `remaster` clean and pushed; Release build and CTest green.
2. Write `docs/releases/<name>.md` (user-facing changes, not included parts, known issues);
   naming `v<original version>-remastered.<n>`, e.g. `v0.9.8v2-remastered.1`.
3. Run the script; check the zip (no `PDAFWPlugin.dll`, no `dlc`, our DLL/launcher hashes in
   `Witcher3VR/BUILD.txt`).
4. Install test from the zip on 5.x (and 4.04 when 4.04 code changed), launcher included, with the
   maintainer's go-ahead for game-directory changes.
5. With the maintainer's go-ahead: tag the commit and create the GitHub release with
   `gh release create <name> --target <commit> --title ... --notes-file docs/releases/<name>.md`
   plus the zip and SHA256 file, marked as pre-release.

When the original project publishes a new release, update `-UpstreamTag`, `-UpstreamAsset` and
`-UpstreamSha256` defaults in the script (asset digest from `gh release view <tag> -R
tig3rmast3r/witcher3-vr --json assets`).

Dev install procedure:
1. Install the official release once into the target game.
2. Replace only the self-built files: `dxgi.dll` goes to
   `bin\x64_dx12\witcher3vr-mod-reference\dxgi.dll`, and the launcher goes next to it in
   `bin\x64_dx12`. The launcher creates the active root copies from the reference folders,
   so a `dxgi.dll` placed directly in `bin\x64_dx12` may be overwritten.
3. Replace changed `support/` scripts in `mods\` as needed.

Windows Defender may flag self-built `dxgi.dll` files as `Trojan:Win32/Wacatac.C!ml` (ML false
positive for an unsigned hooking proxy DLL) and quarantine them from the build output and the
game folder. Add Defender exclusions for the build directory and the installed `dxgi.dll` paths.

## Tests: what they do and do not cover

- `tests/*.cpp`: unit tests of header-only policy modules (eye geometry, AER scheduling,
  HUD/transport policies, etc.). No game involved.
- `tests/*.cmake` "contract" tests: `file(READ)` + `string(FIND)` over source text. They assert
  that code fragments exist or were retired. They check text, not behaviour. Editing an
  asserted fragment breaks them, which may be intended. Update them deliberately.
- `launcher/tests.cpp` etc.: settings/path logic against a fake layout.
- `swapchain_identity_tests` calls `CreateDXGIFactory1` from the build output directory, so
  Windows loads the freshly built proxy `dxgi.dll` next to it. Older builds then wrote
  `[Hidden] uiHorizontalFrameScale/uiVerticalFrameScale` into the real
  `%USERPROFILE%\Documents\The Witcher 3\dx12user.settings`. Since `host_is_game_process()` the
  proxy writes it only inside `witcher3.exe`; builds before that commit still have the side effect.
- **Nothing tests compatibility with a real `witcher3.exe`.** A green `ctest` says nothing
  about whether hooks, offsets or shader hashes match a game build. That needs the smoke test
  and finally a headset run by the maintainer.
- Smoke test: `scripts/smoke-test.ps1 -GameDirectory <bin\x64_dx12> [-Dll <built dxgi.dll>]`
  installs the DLL and input script, points `[test]` in `witcher3vr.ini` at the script (restored
  afterwards), starts the game, waits for it to exit, collects logs and captures under
  `build\smoke-runs\<run>` and writes `summary.txt`. Exit code 0 = passed (script loaded, all
  captures written, no hook failure, no timeout). Default script:
  `scripts/test-input/smoke-continue-remaster.txt` (5.01, ~90 s); 4.04 uses `smoke-continue.txt`.
  The run starts the real game, so it needs the maintainer's go-ahead like any game start.
  Check in the captures that the run reached gameplay: on 4.04 with cloud saves from 5.x in the
  load list, "Continue" picks a 5.x save that 4.04 cannot load, and `smoke-continue.txt` then
  passes while the game stays in the main menu (use a script that picks the local save through
  LOAD GAME). In unattended runs the game window is usually not in the foreground on Windows;
  that is fine.
  `-IniSet 'openxr.enabled=1,openxr.hmd_roll_scale=0'` plus
  `-OpenXrRuntimeJson <SteamVR>\steamxr_win64.json` runs the full OpenXR/dual-render path
  without a headset through the SteamVR null driver (setup in `docs/remaster-port-status.md`,
  2026-10-13). A `flight_dump` script action writes the flight recorders and, with
  `[debug] thread_stack_sampler=1`, a render-thread CPU profile to the log. An `hmd <yaw> <pitch>`
  action turns the located OpenXR head pose (degrees, held until changed), so head-motion bugs can
  be reproduced with the static null HMD. Captures include the
  two images submitted to OpenXR (`*_xr_eye0/1.png`) when a stereo pair is active.
- Linux/Proton smoke test: `linux/w3vr-smoke.py [--dll <built dxgi.dll>] [--repeat N]` does the
  same for a Heroic (GOG) or Steam install: refuses to start unless `[Gameplay] CrossProgression`
  is false, backs up `dxgi.dll`, `witcher3vr.ini` and the saves, starts the game through the
  launcher with `[test] keep_focus=1`, kills it after `--timeout`, collects into
  `build/smoke-runs/<date>_lx<n>_<name>`, writes `summary.txt` and restores everything
  (sha256-checked; changed saves are moved into the run folder). `dx12user.settings` is only
  compared, never restored (the game may refresh the account token on exit). Default script:
  `scripts/test-input/smoke-continue-remaster-linux.txt` (5.01, ~130 s), checked for XR eye images
  in gameplay, a first `Actor.IsInGameplayScene` call (fails when the game never left the main menu) and an
  exit through the menu. Same go-ahead rule as above.
- In-game test mode (`[test]` in `witcher3vr.ini`, logic in `src/test_input_script.h`):
  `record_input=1` records the physical gamepad to `witcher3vr-input-recording.txt`,
  `input_script=<file>` replays a script (same format) through the XInput hook (4.04) or
  GameInput (5.01), also without a controller (5.01: virtual GameInput gamepad; a connected
  gamepad disables it), and its `quit` action closes the game window. Times are ms since the
  first Present. Diagnostic census and inventory files from `[debug]` complement it.
  Under Proton there is no GameInput redistributable; 5.01 then polls XInput, so scripts run
  through the XInput hook there. The game pauses itself when its window loses focus;
  `keep_focus=1` hides that from the game so runs survive clicks into other windows.

## Batching changes and test runs (optional)

A session may develop several changes in one go, by hand or with an AI agent, and test them
together afterwards: one automated run (build, CTest, smoke/null-HMD runs) and one headset session
by the maintainer instead of one per change. Headset time is the scarcest resource, so this is
often the faster way. It is a choice, not a rule; test single changes on their own when that is
simpler.

- Each change is still its own commit that builds, as in the git workflow below. Batching changes
  how often you test, not how you commit.
- Batch changes that touch different areas. When two changes could cause the same symptom (both
  in the stereo pair path, both in the HUD composite), test them separately or plan a bisect.
- Keep a short test list for the batch: what each change needs checked, so one session covers all
  of them (5.01, and 4.04 where the change can affect it).
- If the batched run fails, bisect by commit: every commit builds, so a DLL per commit narrows it
  down headset-free first.
- Record in `docs/remaster-port-status.md` which commits a run covered and which checks are still
  open, the same as for single changes.

## Test state and backups

Tests change the game folder and the game's documents folder (Windows:
`Documents\The Witcher 3`; Proton/Wine: `drive_c/users/<user>/Documents/The Witcher 3` inside the
prefix). The maintainer may also have a real playthrough in that folder. So every test that touches
them follows the same routine, on Windows and on Linux:

1. Ask the maintainer first (see "Porting rules").
2. Back up before changing anything, into a dated folder under `local/` with a `sha256.txt`
   (`sha256sum` / `Get-FileHash` of every file):
   - game folder `bin/x64_dx12`: `dxgi.dll`, `witcher3vr-mod-reference/dxgi.dll`,
     `witcher3vr.ini` (and test scripts or loaders you add) -> `local/game-backups/<purpose>_<date>/`;
   - documents folder: the settings files (`dx12user.settings` and its `.bak` copies,
     `input.settings`, `profile.settings`) -> `local/settings-swap/<purpose>_<date>/`;
   - `gamesaves`, before tests that load, create or change saves -> `local/saves/<purpose>_<date>/`.
     Put test saves in only for the test and restore the previous folder afterwards. Never delete or
     overwrite saves of a real playthrough.
3. After the test, copy everything back and check it against `sha256.txt`. The game rewrites
   `dx12user.settings` when it starts and exits: re-read it before editing it. Once the game has
   run, put back only the keys the test changed instead of the old file, because the game may
   have refreshed the account token in `[Galaxy]` on exit; compare the other keys afterwards.
   Move logs, captures and test scripts the run left in the game folder to `local/runs/<run>/`.
4. Keep a list of backups (what, when, why, whether it can go) in `local/BACKUPS.md`.

Name runs `<date>_<id><number>_<description>` and keep each run's logs and captures in
`local/runs/<run>/` with a short `README.txt` (setup, INI changes, result). `scripts/smoke-test.ps1`
restores the INI it changes; the routine above covers everything else. `linux/w3vr-smoke.py` runs
the routine itself for the game folder and the saves (backups in the run folder, restore checked
against sha256, changed saves moved into the run folder) and only compares `dx12user.settings`.

Pin the game to a supported build on development and test machines and turn off automatic updates:
GOG Galaxy auto-update off (and roll back to the previous version where needed), Heroic "Keep game
at specific version" (for example 5.01), Steam the matching branch and "Only update this game when I
launch it". The build guard only knows the builds in `src/game_build.h`; after a silent update the
mod stays inactive, which looks like a mod bug, and test results are only comparable with the same
build on every machine. Install a new patch deliberately, together with checking or adding its
addresses in `src/engine_addresses.h`.

Turn off the game's cross-progression on development and test machines: `[Gameplay]
CrossProgression=false` in `dx12user.settings` (also in the game's gameplay options). Check it before
every game start, because the game rewrites the file (`linux/w3vr-smoke.py` refuses to start
otherwise). The same goes for settings copies that are swapped into the documents folder, for
example a 4.04 copy on a machine with both versions: the file itself, its `.bak` and the launcher's
`.w3vr.bak` and `.w3vr-original.bak` (*Restore Original Settings* copies the latter back).
Cross-progression syncs saves through the GOG/CD PROJEKT RED account, independently of the cloud
sync of GOG Galaxy, Heroic or Steam: test saves (including autosaves of scripted runs) would
otherwise show up on other machines as the newest save, also in a real playthrough. With 5.x saves
in the cloud, 4.04 also lists them, and "Continue" then picks a save 4.04 cannot load. Move saves
between machines only as files, with the backup routine.

`dx12user.settings` contains a GOG account refresh token (`[Galaxy] tokenRefA`): never commit it and
never sync it automatically. GOG cloud saves (Galaxy, or Heroic on Linux) stay off for
test installs, so that test saves cannot reach a real playthrough through the cloud.

## Launcher behaviour worth knowing

"Configure Settings for VR" (`ConfigureGameSettingsForVr`, `launcher/config.cpp`) replaces
`Documents\The Witcher 3\dx12user.settings` with the bundled `launcher/vr_dx12user.settings`.
Only the `[Galaxy]` and `[Localization]` sections and the camera inversion keys in `[Input]`
(`InvertCameraX/Y`, `...OnMouse`) are preserved. Other user preferences are reset. A one-time
original backup is created and is restored by "Restore Original Settings".

## Architecture (short)

Almost everything is in `src/dxgi_proxy.cpp` (~55k lines). Entry: `DllMain` → `ensure_initialized`
(loads `witcher3vr.ini`, MinHook, then `install_engine_*` hooks). D3D12/DXGI vtable hooks track
resources/PSOs (FNV-1a hash of shader bytecode), game-internal hooks drive the camera/stereo
logic, OpenXR session + D3D12 swapchains submit in Present. Modes: Mono, Stereo (engine frame
factory called twice per Present), AER+AFW (alternate eye + PureDark reprojection). Witcher
scripts in `support/mod*` talk to the DLL through hooked native script functions.
Build-dependent addresses, signatures and layout values: `src/engine_addresses.h`; shader
hashes per build: `src/shader_fingerprints.h`. Details: `docs/remaster-porting-analysis.md`.

## Porting rules

- **Never run unmodified upstream builds on the Remaster.** Upstream has no version check.
  Its game hooks are raw RVAs patched blindly, and six are installed unconditionally at
  startup. This fork's build guard (`src/game_build.h`) disables engine hooks on unknown or
  unsupported builds. Only builds that contain it may be used on 5.01. Progress log:
  `docs/remaster-port-status.md`; current known problems and untested areas:
  `docs/remaster-open-issues.md`.
- Keep the two game installations strictly separate. Never copy mod files into the wrong one.
  Both share `Documents\The Witcher 3` (saves, `dx12user.settings`), so back it up before
  switching versions or using the launcher's "Configure Settings for VR".
- Keep 4.04 working: per-build address tables with 5.01 signature checks, 4.04 values unchanged.
- Agents ask the maintainer before side effects: installs, pushes, deletes, anything in game
  directories or `Documents\The Witcher 3`.
- No credentials or tokens.

## Data hygiene (what may be committed)

Allowed in git: source, tests, and docs whose content is derivable from the public source
(constant names, hook lists, methodology, plans).

Never commit (keep under `local/` or in the local notes file, both git-ignored):
- game files/assets, game or NVIDIA DLLs, save files
- user settings files from `Documents\The Witcher 3`: `dx12user.settings` contains an account
  refresh token (`[Galaxy] tokenRefA`)
- runtime logs (`witcher3vr.log`, OpenXR API dumps, crash dumps), screenshots with HUD/game content
- reverse-engineering artifacts: Ghidra/IDA projects, decompiled code, byte dumps, exe hashes
- machine/user data: local paths, user names, exact hardware identification (driver versions,
  serial numbers, device IDs), AI agent session transcripts

The test setup may be named at the level needed to interpret a result: GPU vendor or model,
headset model, OpenXR runtime, driver module names (e.g. "AMD GPU, Pico 4 over Virtual Desktop").

Address constants and byte patterns needed by the code are fine in source (upstream does the
same). The material they were derived from is not.

## Git workflow

- Remotes: `origin` = the fork (https://github.com/joschi127/witcher3-vr), default push target.
  `upstream` = tig3rmast3r/witcher3-vr, fetch only. Never push to `upstream`.
- `main` mirrors `upstream/main` and gets fast-forwards only, no fork commits.
- `remaster` is the working branch for the port and the fork's default branch on GitHub (its
  README, issue templates and the fork's issues are what visitors see). Commit directly to it in
  small, self-contained commits (easy to cherry-pick for upstream PRs later). Every commit must
  build.
- `backport/*`: fixes offered upstream, based on `main`, one commit each; never merged into
  `remaster`, which carries the same fixes in its own form.
- Which commit was verified in the game (5.01, plus a 4.04 regression check where relevant) is
  recorded in `docs/remaster-port-status.md`, including open checks. That is the reference for
  "known good".
- Separate branches only for experiments that will likely be thrown away.
- **No AI attribution in commits or PRs** (no `Co-Authored-By: <AI agent>` trailers, no
  "generated with" lines). Commits are authored by the repository owner only. The point is not
  to hide the use of AI tools; GitHub would list the tool as a contributor otherwise.
- Check before every push: no files from the data-hygiene "never commit" list, no binaries,
  no local paths or exact hardware identification in the outgoing commits.

## Port roadmap (agreed with the maintainer)

- **Build guard and address tables:** engine hooks run only on recognized builds; 5.01 hooks
  come from `src/engine_addresses.h` with signature checks, enabled hook by hook with log and
  smoke-test verification.
- **Launcher "Configure Settings for VR" must become build-aware.** Whatever is changed by hand
  from the game defaults during the port must later be applied automatically by that button,
  per detected build. This includes settings switched off to stay close to 4.04 (RT/path
  tracing, ray reconstruction, frame generation, Anti-Lag/low latency, HDR, editor preset
  detection) and AA/AO values whose meaning changed in 5.x (`AAMode`: 1 = FXAA on 5.x,
  `SSAOEnabled`). The current 4.04 profile (`launcher/vr_dx12user.settings`) must never be
  applied to 5.x. Keep the list of manual deviations up to date until the launcher implements it.
  Implemented on 2026-10-17: on 5.x the button calls `ConfigureRemasteredGameSettingsForVr`
  (`launcher/config.cpp`), which sets the list below in the existing settings file and keeps
  everything else; new 5.x findings go into that table and this list.

  5.x VR settings the button must write (verified so far; details in
  `docs/remaster-port-status.md`):
  - `[PostProcess] AAMode=1` (FXAA; 5.x values: 0 off, 1 FXAA, 5 FSR, 6 DLSS, 7 XeSS). The mod
    runs the No-AA route on 5.x; TAAU does not exist there.
  - `[PostProcess] AllowShafts=false`: 5.x light shafts veil the whole image whenever the sun is
    in view, which the wide VR field of view makes almost permanent (washed-out colours).
  - Ray tracing off (`[Rendering/RT] EnableRT`, `RTAOEnabled`, `RTGIPreset`, `Shadows`,
    `EnableRtRadiance`, `BalanceModeRT`; `[Rendering/RT/PathTracer] PTEnable`), ray
    reconstruction off (`[PostProcess] EnableDLSSRR`, `[Rendering] AllowDLSSRR`), frame
    generation off (`FSRFramegen=0`, `XessFrameGeneration=false`, `DLSSGMode=0`), low-latency
    modes off (`XeLowLatency=false`, `[Viewport] AMDAntiLag=0`, `ReflexMode=0`), HDR off
    (`[Visuals] HdrEnabled=false`), editor preset detection off
    (`[Rendering] ShouldDetectEditorGraphicsPreset=false`), the game's DLSS off
    (`[Rendering] AllowDLSS=false`, until per-eye upscaling exists), no V-Sync or frame limit
    (`[Viewport] VSync=false`, `[Engine] LimitFPS=0`; the OpenXR runtime paces the frames).
  - Effects that look detached or uncomfortable in VR off, as in the 4.04 profile:
    `[PostProcess] AllowMotionBlur=false`, `AllowBlur=false`, `AllowDOF=false`,
    `AllowLensEffects=false`, `AllowVignette=false`, `AllowChromaticAberration=false`;
    `[Rendering] SSREnabled=false`, `HairWorksLevel=0`. Motion blur is the suspected cause of
    a whole image smeared towards the centre on the Linux test, where the setup tool did not turn
    it off (two eyes per game tick).
  - The full list the launcher writes is `kSettings` in `ConfigureRemasteredGameSettingsForVr`
    (`launcher/config.cpp`); other setup tools (the Linux tool) should write the same list.
  - The colour-blindness filter must stay off (the mod's replacement HUD composite does not
    reproduce it).
  - Per-eye render size = the OpenXR runtime's recommended size (INI `render_width/height`; the
    mod forces it on 5.x too); `native_stereo` per runtime (symmetric FOVs: 0).
  - Keep the user's camera inversion keys, as the 4.04 button does.

## Architecture decision records

Decisions that shape the code or the project and are hard to reverse are recorded as ADRs in
[docs/adr/](adr/README.md): one Markdown file per decision with context, decision and
consequences. Write one when a choice affects architecture, game-version support, scope,
compatibility or the workflow, when it keeps coming up again, or when the maintainer asks for
one. Bug fixes, tuning values and test results do not need ADRs; they belong in the commit
message and `docs/remaster-port-status.md`.

These guidelines stay the short rule set and link to an ADR where the reasons matter. An accepted
ADR is not rewritten later: a change of mind is a new ADR that supersedes the old one. Open
questions are ADRs with status Proposed until the maintainer decides.

## Open decisions

- One codebase for 4.04 and the Remaster or a separate Remaster-only fork:
  [ADR 0002](adr/0002-one-codebase-for-404-and-5x.md) (Proposed).

## Upstream relationship

- Upstream currently supports only Next-Gen 4.04 (latest Next-Gen update), not the Remaster.
  CONTRIBUTING's "latest official PC version only" rule predates the Remaster and should not
  be read as Remaster support. The upstream README (2026-10-13) says Remastered support is not
  yet planned.
- Remaster support was asked about upstream in
  [issue #27](https://github.com/tig3rmast3r/witcher3-vr/issues/27); on 2026-10-13 the maintainer
  reported the first playable 5.01 version there and asked how to proceed (fork review, summary
  or PR). Check it for news before larger work. On 2026-10-14 the author replied there that he
  had not looked at the code yet, is open to the fork continuing, is unsure about supporting both
  game versions in one mod, and sees the Remaster as a chance to clean up the code.
- On 2026-10-07 the author had no plan yet for a Remaster port, "not this month" (#27). In
  [#31](https://github.com/tig3rmast3r/witcher3-vr/issues/31) he wrote that he would not add or
  change anything else in the Next-Gen version, but later released Next-Gen hotfixes, so this is
  not settled. See `docs/remaster-research.md` for the public information collected so far.
- Fixes found in the fork that also apply to 4.04 are offered upstream as issues with backport
  branches (`backport/*`, based on `main`, never merged into `remaster`):
  [#33](https://github.com/tig3rmast3r/witcher3-vr/issues/33) (Full VR cutscenes: head-locked
  sky/trees and camera lights, posted 2026-10-17).
- Upstream's scope rules apply to bug reports: do not report problems from this fork or from
  Remaster builds upstream as regular bugs.
- Coordination with the upstream author is the maintainer's decision; agents never contact
  upstream on the maintainer's behalf. Keep changes focused and upstream-friendly: small
  commits, English, upstream code style, no unrelated refactors.
