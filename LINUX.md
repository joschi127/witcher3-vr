# Witcher 3 Remastered VR on Linux

This guide covers running the mod on Linux under Proton, started from Steam or from Heroic Games
Launcher (GOG edition), with an OpenXR runtime such as WiVRn on the host. The main target is
**The Witcher 3 Remastered, patch 5.01**. Patch 4.04 is covered briefly
[at the end](#patch-404-next-gen-branch).

`linux/w3vr-linux.py` replaces the Windows launcher on Linux. It is Python 3 with the standard
library only, so it also runs on SteamOS. The [README](README.md#linux-proton) has the short
version of these steps.

> **State in this repository.** This guide and the Linux tools come from
> [kaarele/witcher3-remastered-vr](https://github.com/kaarele/witcher3-remastered-vr) (tested there with an NVIDIA GPU, Proton and WiVRn on a
> Quest 3) and are being taken over feature by feature. Until the per-eye upscaler history is taken
> over, Remastered 5.01 runs with FXAA here: `configure` uses `--aa none` on 5.01 and refuses
> `--aa dlss` and `--aa fsr`, which would otherwise blend both eyes into one history. Keys that
> `configure` writes for features not taken over yet are ignored by the mod. The tool installs the
> state-bridge variant of this repository's releases (`variants/remastered` or `variants/nextgen`),
> as the Windows launcher does, and `install --sha256` also accepts this repository's release
> checksum files (`sha256sum` format).

The setup is the same for Steam and Heroic except for where the launch settings go: a launch-option
line in Steam, environment variables and a sandbox permission in Heroic (see
[Launcher setup](#launcher-setup)).

## Requirements

- **Steam or Heroic Games Launcher, with Proton.**
  - Steam: Proton Hotfix is tested (in kaarele's project). The tool finds native Steam, Flatpak
    Steam and the Steam Deck layout, and every Steam library.
  - Heroic (native or Flatpak) with the GOG edition: GE-Proton 11 is tested, Proton-CachyOS without
    a headset.
  - For any other install, every command takes `--game-dir DIR --prefix WINEPREFIX`.
- **The Witcher 3 Remastered, patch 5.01, DirectX 12.** The mod checks the game build at startup
  and does nothing on other builds.
  - Steam: the default branch.
  - Heroic: the GOG edition, kept at 5.01 (automatic updates off).
- **An OpenXR runtime on the host.** WiVRn (Flatpak `io.github.wivrn.wivrn`) is tested, with a
  Quest 3 (kaarele's project) and a Pico 4 (this repository). The tool also recognises Monado and
  SteamVR as the active runtime, but they are untested.
- **Python 3.** SteamOS, Bazzite and most desktop distributions include it.
- **A `dxgi.dll` built from this repository.** The Linux tests of this repository use a DLL built
  on Linux, as described in [linux/BUILD-LINUX.md](linux/BUILD-LINUX.md); the result is
  `build/linux-clang-cl/dxgi.dll`.
- **A Witcher 3 VR Remastered release zip** from
  [joschi127/witcher3-vr releases](https://github.com/joschi127/witcher3-vr/releases). It supplies
  `openxr_loader.dll` and the base script mods. Check it against its SHA-256 file with
  `sha256sum -c <name>-SHA256.txt`.
- **For DLSS:** an NVIDIA RTX GPU. Other GPUs use FXAA (tested on an AMD integrated GPU).
- Mouse and keyboard, or a gamepad. VR motion controllers are not supported.

## Before you start

- **Back up your saves.** `install` also makes a verified backup first (see
  [backup-saves](#backup-saves)). The saves are in `drive_c/users/steamuser/Documents/The Witcher
  3/gamesaves` inside the game's Wine prefix (`status` prints the prefix):
  - Steam: the prefix is `steamapps/compatdata/292030/pfx` in the Steam library that holds the
    game.
  - Heroic: the prefix is set in the game's Heroic settings (*Wine*); with Proton it is the `pfx/`
    folder inside it.
- **Start the game once without the mod**, reach the main menu and quit. This creates the Proton
  prefix and the settings files the tool edits (`dx12user.settings` and `input.settings`).
- **Close the game** before you run `install`, `configure` or `uninstall`. The game rewrites its
  settings when it exits, and the tool refuses to run while it is running.

## WiVRn and the headset

1. **Install WiVRn:** `flatpak install --user flathub io.github.wivrn.wivrn` (per user; a
   system-wide install works too). Start it and follow its setup assistant.
2. **Install the WiVRn app on the headset.** Quest headsets get it from the store. For a Pico the
   assistant installs the APK over USB with `adb`. Server and headset app must be the same version.
   - Pico 4: turn on developer mode (*Settings > General > About*, tap *Software Version* several
     times), then *Settings > Developer > USB Debugging*. Accept the debugging prompt in the
     headset when you plug in the cable.
   - Use a USB data cable, connected to the PC directly. With a charge-only cable, or behind some
     docks, the headset does not show up at all (`adb devices` stays empty).
   - `adb` needs udev rules for the headset. Debian's `android-sdk-platform-tools-common` includes
     them for Pico (USB vendor `2d40`).
3. **Connect the headset to WiVRn before you start the game.** WiVRn only registers itself as the
   active OpenXR runtime (`$XDG_CONFIG_HOME/openxr/1/active_runtime.json`) while a headset is
   connected, and Proton's VR setup and the mod's OpenXR start-up both run once, at game launch.
   `status` shows the active runtime.

WiVRn's dashboard also shows a launch-option line for Steam games; the tool prints the matching
settings for your launcher, see [Launcher setup](#launcher-setup).

## Install with w3vr-linux.py

Run these from a checkout of this repository. The tool reads the launcher templates
(`launcher/`), the 5.01 script variants (`support/remaster/`) and the Linux extras
(`linux/extra-mods/`) relative to its own location.

```bash
python3 linux/w3vr-linux.py status
python3 linux/w3vr-linux.py install <release>.zip --dll build/linux-clang-cl/dxgi.dll
python3 linux/w3vr-linux.py configure
python3 linux/w3vr-linux.py launch-options
python3 linux/w3vr-linux.py wivrn-shortcut        # optional, Steam only
```

Then add what `launch-options` prints to your launcher, as described in
[Launcher setup](#launcher-setup). `python3 linux/w3vr-linux.py <command> --help` lists the options
of each command; [Commands](#commands) describes them.

## Launcher setup

Both launchers run the game inside Steam's runtime container (Heroic through umu). Proton's
`wineopenxr` passes the game's OpenXR calls (D3D12) to the host runtime, so the container must
import the host's runtime (`PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES=1`). Nothing is needed inside the Wine
prefix: Proton registers `wineopenxr` as the prefix's OpenXR runtime by itself.

### Steam

`launch-options` prints the line for
*Steam > The Witcher 3 > Properties > General > Launch Options*. With WiVRn installed as a system-wide Flatpak:

```text
PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES=1 PRESSURE_VESSEL_FILESYSTEMS_RW=/var/lib/flatpak/app/io.github.wivrn.wivrn %command% --launcher-skip
```

- The two `PRESSURE_VESSEL_*` variables let the game, inside Steam's runtime container, reach the
  OpenXR runtime on the host. `PRESSURE_VESSEL_FILESYSTEMS_RW` is only added for a Flatpak WiVRn;
  for a per-user Flatpak install it points to `~/.local/share/flatpak/app/io.github.wivrn.wivrn`.
- `--launcher-skip` makes the REDprelauncher start the DirectX 12 game directly.
- If your runtime shows its own launch string (for example the WiVRn dashboard), prefer that, and
  keep `%command% --launcher-skip`.
- `WINEDLLOVERRIDES="dxgi=n,b"` and `PROTON_ENABLE_NVAPI=1` can be added. Proton Hotfix already
  sets both for this game on NVIDIA systems; on other Proton versions they make sure the mod's
  `dxgi.dll` loads and NVAPI is available for DLSS.

### Heroic (GOG)

Tested with Heroic 2.22 (Flatpak) and the GOG edition on patch 5.01: under GE-Proton 11 with WiVRn
(per-user Flatpak) and a Pico 4, and under GE-Proton 11 and Proton-CachyOS without a headset
(OpenXR through the null runtime from `tools/w3vr-nullxr`). The tool finds the install and its Wine
prefix from Heroic's settings by itself and does not change Heroic's settings.

1. Install the game in Heroic and keep it on patch 5.01: turn off automatic updates for it. The mod
   does nothing on other builds.
2. Choose a Proton build: in Heroic's *Wine Manager*, the GE-Proton builds are on their own tab at
   the top of the page; download *GE-Proton-latest*, then select it under *The Witcher 3 >
   Settings > Wine*. Proton-CachyOS works as well. With Proton the tool uses the prefix's `pfx/`
   folder.
3. Turn off cloud saves for the game in Heroic while you test.
4. Start the game once without the mod and quit from the main menu, then run `install` and
   `configure` as above.
5. Run `launch-options` and add what it lists under *Heroic > The Witcher 3 > Settings > Advanced*;
   it marks what is already set:
   - environment variables: `WINEDLLOVERRIDES` = `dxgi=n,b`, and for a headset
     `PRESSURE_VESSEL_IMPORT_OPENXR_1_RUNTIMES` = `1`;
   - game argument: `--launcher-skip`, which makes the GOG build's REDprelauncher start the
     DirectX 12 game directly.
6. **Flatpak Heroic only, for a headset:** let Heroic's sandbox see the OpenXR runtime.
   `launch-options` prints the command for your install; for a per-user WiVRn Flatpak:

   ```bash
   flatpak override --user --filesystem=xdg-config/openxr:ro \
     --filesystem=xdg-data/flatpak/app/io.github.wivrn.wivrn:ro \
     --filesystem=xdg-run/wivrn com.heroicgameslauncher.hgl
   ```

   These are the active-runtime file, the WiVRn runtime and the WiVRn server's socket. Quit Heroic
   completely and start it again, so that the override applies.

To play without the mod, remove `WINEDLLOVERRIDES` again or run `uninstall`.

## Commands

### status

Shows what the tool found, without changing anything:

- the launcher and the game folder:
  - Steam: the Steam root (and whether it is Flatpak Steam), the Steam branch and build id;
  - Heroic: Heroic's settings folder, the Wine prefix, the Proton build and the game's
    `WINEDLLOVERRIDES`
- the `bin/x64_dx12/witcher3.exe` build: `Remastered 5.01`, `4.04 (Steam next-gen)`, or
  `UNKNOWN build` with its TimeDateStamp and SizeOfImage
- the prefix's `Documents/The Witcher 3` folder
- whether the mod is active (`bin/x64_dx12/dxgi.dll` exists). With `install --dll` it reports
  `differs from reference`, because your DLL replaces the release's reference DLL.
- the active OpenXR runtime, whether an NVIDIA GPU was found, and whether the game is running
- a summary of the current `witcher3vr.ini` and `dx12user.settings` VR keys

### install

```bash
python3 linux/w3vr-linux.py install ZIP [--dll FILE] [--sha256 FILE] [--force]
```

- Checks that `witcher3.exe` is a supported build (5.01 or 4.04). `--force` installs anyway.
- Backs up the prefix's `Documents/The Witcher 3` folder (saves and settings) and verifies the copy.
- Extracts the zip into the game folder. It must contain only `bin/`, `mods/`, `dlc/` and
  `Witcher3VR/` at the top level; unsafe paths are refused and backslash paths are handled.
- Any existing file it overwrites is copied to `$XDG_DATA_HOME/w3vr-linux/overwritten/` (default
  `~/.local/share/w3vr-linux/`) first, and `uninstall` puts it back.
- Activates the renderer: copies `bin/x64_dx12/witcher3vr-mod-reference/dxgi.dll` to
  `bin/x64_dx12/dxgi.dll`, or the file given with `--dll`.
- Installs `linux/extra-mods/modW3VRKeepHudFrameScale` (keeps the VR HUD frame scale when the
  game's *Rescale HUD* screen opens).
- On 5.01, installs the 5.01 state-bridge script from `support/remaster/` (the 4.04 one does not
  compile on 5.01) and adapts the state names in the installed script to the detected build.
- `--sha256 FILE` checks the zip against a checksum file in the upstream `key=value` format
  (`main_archive.sha256=<hex>`, and `dll.sha256=<hex>` for the reference DLL), as shipped with
  the 4.04 releases. For a file in `sha256sum` format, run `sha256sum -c` yourself before
  installing. Without `--sha256` the tool prints a warning and does not verify the archive.

To update the DLL later, run `install` again with the same zip and the new `--dll`.

### configure

```bash
python3 linux/w3vr-linux.py configure [options]
```

Writes `bin/x64_dx12/witcher3vr.ini`, the VR keys in `dx12user.settings` and the HUD editor key
bindings in `input.settings`. It needs the mod installed and `dx12user.settings` present. An
existing `witcher3vr.ini` is updated and keeps your other keys; otherwise the tool starts from
`launcher/witcher3vr.default.ini`. Every file it changes keeps its previous version as
`<file>.w3vr.bak`, and the first run also saves `dx12user.settings.w3vr-original.bak`.

| Option | Default | What it does |
|---|---|---|
| `--mode stereo\|mono\|aer` | `stereo` | `stereo` = strict Stereo, both eyes from one game tick. `mono` = Mono. `aer` = AER+AFW (4.04 only; faster, but can shimmer). |
| `--aa dlss\|none\|taau\|fsr` | `none` on 5.01; on 4.04 `dlss` on NVIDIA, otherwise `taau` | `none` = FXAA on 5.01. `dlss` = the game's DLSS with a separate history per eye. `taau` exists on 4.04 only. On 5.01 `dlss` and `fsr` are refused until the per-eye history is taken over. |
| `--dlss-quality 0-4` | `1` | `1` Quality, `2` Balanced, `3` Performance, `4` Ultra Performance, `0` DLAA. On 5.01 the in-game DLSS setting also changes it while playing. |
| `--res WIDTHxHEIGHT\|runtime` | `2476x2594` | Per-eye render size. The default is WiVRn's recommended size for a Quest 3; for another headset pass your runtime's recommended per-eye size (WiVRn with a Pico 4: `2592x2592`), or `runtime` to use the runtime's recommendation at every start. |
| `--hud-size` | `0.5` | Size of the VR HUD plane in Stereo (`0.5` = half size). |
| `--hud-frame` | `1.0` | The game's HUD frame scale (`1.0` = vanilla; lower values pull the panels toward the centre). |
| `--hud-locked 0\|1` | `1` | `1` = the HUD stays in front of you; turn your head to look at it. `F9` recentres. |
| `--near-view OFFSET` | `-1.0` | Start in the near view with this camera offset. Negative values move the camera back, positive values closer. |
| `--no-near-view` | | Start in the game's standard camera instead. |
| `--symmetric` | | 5.01: render the symmetric envelope and crop it (older projection) instead of per-eye frusta. Per-eye frusta are not taken over yet, so this changes nothing here. |
| `--world-detail` | `0.7` | `0.4`-`1.0`, lower = less CPU per eye. Affects 4.04 only. |
| `--diagnostics` | off | Turn on the mod log and diagnostics. They cost frame rate; use them only when you need logs. |
| `--reset-settings` | | 4.04: apply the VR graphics template to `dx12user.settings` again. |
| `--force` | | Configure even if `witcher3.exe` is an unknown build (applies the 4.04 preset). |
| `--dry-run` | | Show which files would change, without writing them. |

What it writes is listed in [Settings written by configure](#settings-written-by-configure).

### launch-options

Prints the launch settings for the active OpenXR runtime. `--runtime wivrn|steamvr` picks a runtime
instead of the active one. What to do with the output is described in
[Launcher setup](#launcher-setup).

- Steam: the launch-option line.
- Heroic: the environment variables and the game argument, and for Flatpak Heroic the
  `flatpak override` command, each marked when it is already set.

### wivrn-shortcut

```bash
python3 linux/w3vr-linux.py wivrn-shortcut [--remove] [--autostart 0|1]
```

- Adds a desktop entry (`$XDG_DATA_HOME/applications/witcher3-vr-wivrn.desktop`, category
  `X-WiVRn-VR`) so *The Witcher 3 VR* shows in WiVRn's in-headset app list while no VR app is
  running. It starts the game through Steam (`flatpak run com.valvesoftware.Steam` for Flatpak
  Steam), so it is not available for Heroic installs. `--remove` deletes the entry.
- `--autostart 1` also sets WiVRn's `application` option, so the game starts whenever the headset
  connects. `--autostart 0` removes it. Restart the WiVRn server to apply.

### backup-saves

Copies the prefix's whole `Documents/The Witcher 3` folder (saves and settings) to
`~/w3vr-backups/<timestamp>/` and verifies every file byte for byte. `install` does the same
automatically.

### uninstall

```bash
python3 linux/w3vr-linux.py uninstall [--restore-settings]
```

- Removes the files `install` recorded (without a record, only `bin/x64_dx12/dxgi.dll`), puts
  back the files it had overwritten and removes folders that are left empty.
- Removes the HUD editor line from `dx12filelist.txt` (4.04) and renames `witcher3vr.ini` to
  `witcher3vr.ini.uninstalled`. The HUD editor key bindings stay in `input.settings`.
- `--restore-settings` restores `dx12user.settings` from `dx12user.settings.w3vr-original.bak`.
- Saves are not touched. Remove the launch settings afterwards:
  - Steam: clear the launch options.
  - Heroic: remove the environment variables and the game argument, and the `flatpak override`
    paths if you added them (`flatpak override --user --reset com.heroicgameslauncher.hgl` removes
    all your overrides for Heroic).

Deleting `bin/x64_dx12/dxgi.dll` alone also switches the mod off: Proton loads the native
`dxgi.dll` from the game folder whenever it exists.

## Settings written by configure

**`bin/x64_dx12/witcher3vr.ini`** (defaults shown):

| Key | Value | Notes |
|---|---|---|
| `[openxr] enabled`, `mode` | `1`, `3` | Stereo (`1` with `--mode mono`) |
| `[openxr] mode3_aer_presentation` | `0` | Strict Stereo (`1` with `--mode aer`) |
| `[openxr] resolution_auto`, `render_width`, `render_height` | `0`, from `--res` | |
| `[openxr] native_stereo` | `0` on 5.01, `1` on 4.04 | 5.01 uses `remaster_asymmetric_ndc` instead |
| `[engine] remaster_asymmetric_ndc` | `1` (5.01) | Per-eye projection; `0` with `--symmetric` (not taken over yet, ignored) |
| `[openxr] world_detail_range` | `0.700` | `--world-detail` |
| `[openxr] hud_size` | `0.500` | `--hud-size` |
| `[openxr] hud_stereo_shift_px` | `-72` | `round(-36 / hud_size)`: keeps the HUD plane at about 1 m |
| `[openxr] hud_horizontal_scale`, `hud_vertical_scale` | `1.000` | `--hud-frame` |
| `[openxr] hud_controller_locked` | `1` | `--hud-locked` |
| `[engine] dual_render_probe`, `dual_render_start` | `1` | `0` in Mono |
| `[engine] temporal_backend` | `dlss` or `none` | From `--aa` |
| `[engine] dlss_dlaa` | `0` | `1` only on 4.04 with `--dlss-quality 0` |
| `[engine] menu_state_probe` | `1` | |
| `[engine] camera_mode`, `close_camera_offset` | `1`, `-1.000` | `--near-view`; `--no-near-view` writes `camera_mode=0` |
| `[debug]` logging and diagnostics keys | `0` | `1` with `--diagnostics` |
| `[reverse]` and `[engine]` research probes | `0` | |
| `[test] openxr_loader`, `input_script` | cleared | Test-harness keys must be empty for headset play |
| `[launcher] integration_mode` | `off` | |
| `[meta] config_version` | `19` | |

**`Documents/The Witcher 3/dx12user.settings`** in the Proton prefix:

| Key | Value |
|---|---|
| `[Viewport] Resolution` | `"WIDTHxHEIGHT"` from `--res` |
| `[PostProcess] AAMode` | `6` = DLSS, `1` = FXAA on 5.01 (`0` = none, `3` = TAAU on 4.04) |
| `[PostProcess] DLSSQuality` | 5.01: `1`-`4`, `5` = DLAA. 4.04: `1`-`4` (DLAA through `dlss_dlaa`) |
| `[Rendering] AllowDLSS` | `true` with DLSS |
| The Windows launcher's 5.x list (`REMASTER_VR_SETTINGS`, same as `kSettings` in `launcher/config.cpp`) | 5.01: ray and path tracing, ray reconstruction, frame generation, low-latency modes, VSync, FPS limit, HDR and editor preset detection off; light shafts, motion blur, blur, depth of field, lens effects, vignette, chromatic aberration, screen-space reflections and HairWorks off |
| `[Rendering] AllowDLSSG`, `AllowReflex`, `AllowDLSSRR`; `[PostProcess] EnableDLSSRR` | `false` (5.01). The mod presents its own eye pairs. |
| `[PostProcess] SharpenAmount`; `[Rendering/TAA] EnableCAS` | `0`, `false` (5.01). Sharpening adds shimmer after the stream's resampling. |
| `[Gameplay] EnableAlternateExplorationCamera`, `EnableAlternateCombatCamera` | `1` (5.01): the *Close* cameras |
| `[W3VRSettings] CameraFollowPolicy`, `HideStaticHudOutsideCombat`; `[Gameplay] AutoCameraCenter` | `1`, `false`, `false` |
| `[DLC] DlcEnabled_movementinputfix` | `1` if `dlc/dlcmovementinputfix` is installed |
| `[Hidden] uiHorizontalFrameScale`, `uiVerticalFrameScale` | `--hud-frame` |

On 5.01 only these keys change. On 4.04 the first `configure` (or `--reset-settings`) replaces the
file with `launcher/vr_dx12user.settings`, keeping your `[Galaxy]` and `[Localization]` sections
and your difficulty, as the launcher's *Configure Settings for VR* does.

**HUD editor:** `configure` adds the `Insert` and `F7` bindings to the gameplay contexts and the
`[W3VRHudEditor]` section of `input.settings`. On 4.04 it also registers
`modWitcher3VRHUDEditor.xml` in `bin/config/r4game/user_config_matrix/pc/dx12filelist.txt`; 5.01
has no such file and loads every menu XML in that folder. If `input.settings` does not exist yet,
start the game once and run `configure` again.

## How it works under Proton

- **The mod's `dxgi.dll` proxy loads as on Windows.** Proton loads the native `dxgi.dll` from
  `bin/x64_dx12` first, and the proxy chains to DXVK's dxgi and vkd3d-proton's d3d12.
- **OpenXR goes through Proton's `wineopenxr`.** It bridges the D3D12 session
  (`XR_KHR_D3D12_enable`) to the host OpenXR runtime through vkd3d-proton. The mod's Windows
  `openxr_loader.dll` finds wineopenxr through the prefix registry key
  `HKLM\Software\Khronos\OpenXR\1\ActiveRuntime` (`C:\openxr\wineopenxr64.json`), which Proton
  writes at launch.
- **The Windows launcher is not needed.** `Witcher3VRLauncher.exe` only copies the reference
  `dxgi.dll`, writes `witcher3vr.ini` and `dx12user.settings`, registers the HUD editor and starts
  the game. `w3vr-linux.py` does the same file work.

## Remastered 5.01 notes

- **DLSS under Proton.** The game skips its Streamline (DLSS) setup when it detects Wine. With
  `[engine] temporal_backend=dlss` the mod patches that check in memory at startup (exact 5.01
  build only; it changes nothing on Windows). `[engine] remaster_sl_wine_bypass=0` turns it off.
- **Camera.** The 5.01 default exploration camera (*Modern*) re-frames while walking. With the game
  pitch locked for VR only the camera translation remains, so the view seems to zoom in and out.
  The *Close* preset (*Options > Gameplay > Exploration camera*), which `configure` selects, does
  not do this.

**Not taken over yet.** kaarele's project has these as well; in this repository the keys are
ignored until they are taken over (see the note at the top):

- **DLSS per eye.** In strict Stereo both eyes go through one DLSS state. The mod gives each eye
  its own Streamline viewport, and with it its own DLSS history and motion-vector camera history;
  otherwise DLSS would blend the two eyes. 5.01 has no TAAU and no AER mode.
- **Projection.** With `[engine] remaster_asymmetric_ndc=1` each eye is rendered with its own
  off-centre field of view (through a projection offset that the 5.01 engine applies itself), and
  the whole image goes to the headset. Without it, 5.01 renders a wider centred image per eye and
  the headset only uses a crop of it (about 1993x2089 of 2476x2594), so about a third of the
  rendered pixels are discarded and the image is less sharp. `configure --symmetric` selects the
  older way.
- **HUD.** A `hud_size` below 1 is drawn with filtering and shrinks toward the centre of the
  visible image; the controller-locked HUD is levelled (heading only). Menus and loading screens
  are centred on the visible image too. INI switches: `hud_filtered_sampling`, `hud_anchor_level`,
  `hud_scale_origin=visible|render`, `menu_vertical_center=visible|forward`.

## Performance

- **The game is CPU-limited in VR** (render thread plus game tick), because strict Stereo renders
  the two eyes one after another: two Presents per pair and one `xrWaitFrame` per pair. The
  foliage visibility range and grass density cost the most; lower them first.
- **Frame rates move in steps.** Each pair is shown for a whole number of display refreshes, so a
  steady rate is 45, 30 or 22.5 pairs/s at 90 Hz (40, 30 or 24 at 120 Hz). The Steam FPS counter
  shows twice the pair rate. See
  [Why the frame rate moves in steps](https://github.com/kaarele/witcher3-remastered-vr#why-the-frame-rate-moves-in-steps) in kaarele's README.
- **The headset refresh rate is set in the WiVRn headset app.** The
  [recommended settings](https://github.com/kaarele/witcher3-remastered-vr#recommended-settings) in kaarele's README cover refresh rate and stream
  quality.
- **Keep diagnostics off for play** (`[debug] logging_enabled=0`, `runtime_diagnostics=0`). They
  cost a noticeable share of the frame rate.

## Manual setup (without the tool)

The tool is the supported way. To do the same by hand, with the game closed:

1. Extract the release zip and copy `bin/`, `mods/`, `dlc/` and `Witcher3VR/` into the game
   folder. Copy your `dxgi.dll` to `bin/x64_dx12/dxgi.dll`.
2. Copy the contents of `support/remaster/` and `linux/extra-mods/` into the game's `mods/`
   folder.
3. Create `bin/x64_dx12/witcher3vr.ini` from `launcher/witcher3vr.default.ini` and set the keys in
   [Settings written by configure](#settings-written-by-configure). Do not write a minimal ini:
   several code defaults are off, for example `hmd_freelook`, `menu_state_probe` and
   `dual_render_*`.
4. Set the `dx12user.settings` keys from the same section.
5. Add the HUD editor bindings to `input.settings`. `HUD_EDITOR_BINDINGS` and `HUD_CONTEXTS` in
   `linux/w3vr-linux.py` (and the launcher's `launcher/config.cpp`) list the exact lines.
6. Set up your launcher as shown under [Launcher setup](#launcher-setup).

## Patch 4.04 (next-gen branch)

The tool also recognises Patch 4.04 and applies the 4.04 preset from the original mod. This
project develops and tests 5.01; 4.04 is not tested with this repository's DLL on Linux. The steps
below are for Steam.

- **Saves:** saves from Remastered 5.x cannot be loaded in 4.04, and 4.04's single quicksave slot
  and rotating autosaves delete 5.x quicksaves and autosaves in the same folder. Back up first.
- Switch the game: *Steam > The Witcher 3 > Properties > Game Versions & Betas > `next-gen`*.
  Start it once and quit.
- Install an upstream [witcher3-vr](https://github.com/tig3rmast3r/witcher3-vr) 4.04 release:
  `python3 linux/w3vr-linux.py install Witcher3VR-<version>-Main.zip --sha256 Witcher3VR-<version>-SHA256.txt`.
  Without `--dll` the release's reference DLL is activated.
- `configure` on 4.04 applies the VR graphics template, defaults to TAAU on non-NVIDIA GPUs, offers
  `--mode aer`, and refuses scaled DLSS at near-square render sizes (`|width - height|` below 48),
  as the launcher does.
- **Known issue:** with the upstream 4.04 DLL the pause menu jitters left and right in strict
  Stereo, because untagged menu frames used the Present parity for the eye offset. Fixed in this
  repository's DLL.

## Troubleshooting

**The game does not enter VR.**
- Connect the headset to WiVRn before you start the game, then check `status` for the runtime.
- Check the launch settings (run `launch-options` again):
  - Steam: the launch options.
  - Heroic: the environment variables and, with Flatpak Heroic, the sandbox override. Restart
    Heroic after changing the override.
- Make sure `[test] openxr_loader` in `witcher3vr.ini` is empty (`configure` clears it).

**The mod does nothing, or `status` reports an `UNKNOWN build`.** The game was probably updated.
The mod switches its hooks off on builds it does not know. For details run
`python3 -I linux/pe-build-id.py "<game>/bin/x64_dx12/witcher3.exe"`: it prints the exe's
TimeDateStamp, SizeOfImage and FileVersion, the build the mod maps them to, and checks the 5.01
engine function signatures (`--scan` also searches the whole image for each one). Include its
output in bug reports.

**Script compilation error at game start.** The 4.04 state-bridge script is installed on 5.01.
Run `install` again, or copy `support/remaster/modWitcher3VRStateBridge` into `mods/`.

**`error: The Witcher 3 is running`.** Quit the game. The tool looks for a running `witcher3.exe`
and refuses to change files while it runs.

**`dx12user.settings is missing` or `input.settings not found yet`.** Start the game once without
VR, reach the main menu and quit, then run `configure` again.

**DLSS is missing or has no effect.** Use `--aa dlss` (`[engine] temporal_backend=dlss`) with DLSS
as the game's anti-aliasing, an RTX GPU and NVAPI enabled (`PROTON_ENABLE_NVAPI=1`), and make sure
`[engine] remaster_sl_wine_bypass` is not `0`.

**The HUD frame scale resets after *Options > Video > Rescale HUD*.** That screen forces a value of
at least 0.9, and just opening it sends the value. `modW3VRKeepHudFrameScale` (installed by the
tool) keeps the VR value. With `--hud-frame 1.0` there is nothing to override.

**The desktop window is very tall.** It takes the per-eye render size, so it can be taller than
your monitor and the Steam FPS counter in its corner can be off-screen. This is harmless.

**Slow loading, or the automatic reload after death hangs.** Both also happen with vanilla 5.01
under Proton. Quit and load the save manually.

**Logs.**
- With `configure --diagnostics` (`[debug] logging_enabled=1` and `runtime_diagnostics=1`) the mod
  writes `bin/x64_dx12/witcher3vr.log`.
- Steam: `PROTON_LOG=1` in the launch options writes `~/steam-292030.log`.
- A successful start logs `OpenXR xrCreateSession(D3D12) result=XR_SUCCESS` and
  `OpenXR session state changed=5` (focused).
- Developers: `linux/tools/camtrace.py witcher3vr.log` summarises the camera trace written with
  `[debug] camera_offset_trace=1`.

## Building the DLL on Linux

`linux/build/` holds a clang-cl and lld-link toolchain that runs inside a distrobox. Microsoft's
CRT and Windows SDK come from `xwin`, which requires you to accept Microsoft's licence. The
dependency script is a bash port of `setup-dependencies.ps1`. See
[linux/BUILD-LINUX.md](linux/BUILD-LINUX.md).
