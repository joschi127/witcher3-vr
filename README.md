# The Witcher 3 VR

Play The Witcher 3 Next-Gen in VR with configurable stereo rendering, a movable VR HUD, first-person gameplay, cinema modes, DLSS, DLAA and TAAU support, optional frame generation, ReShade and OptiScaler integration.

Its DX12 and VR architecture was informed by
[REFramework](https://github.com/praydog/REFramework) and
[UEVR](https://github.com/praydog/UEVR) by praydog; neither project is bundled
as a runtime dependency.

> [!WARNING]
> This project is under active development. Features may be incomplete,
> unstable, or incompatible with some hardware and game configurations.
> Intermittent crashes may occur when using DLSS 5 Neural Rendering.
> This release includes safeguards, but does not resolve every case.

> [!IMPORTANT]
> The included custom OptiScaler is the only launcher-managed DLSS 5 integration.
> Supply only `nvngx_dlssnr.dll` separately. ReShade is included without add-ons;
> user-installed DLSS 5 ReShade add-ons are not supported.

> [!IMPORTANT]
> Gameplay currently requires a mouse and keyboard or a gamepad. VR motion
> controllers are not supported and are not currently planned at this time.
> Remastered version support too is not yet planned.

## Highlights

### Three ways to play

| Mode | What it offers | Anti-aliasing and upscaling |
| --- | --- | --- |
| AER + AFW | The fastest stereo option. Best when you need the highest frame rate. | TAAU, DLSS or DLAA |
| Stereo | Both eyes are rendered every frame. Better image consistency at a higher performance cost. | No AA, FXAA, TAAU, DLSS or DLAA |
| Mono | The lightest mode. It has no stereo depth, but can be useful on slower systems. | No AA, FXAA, TAAU, DLSS or DLAA |

Asymmetric projection is enabled by default. Press `F2` to switch temporarily to symmetric projection if needed. This toggle is intended for testing and may be removed in a future release.

### Frame Generation for VR

OFXR is a VR frame-generation system based on optical flow. It can improve smoothness when the game cannot reach the headset refresh rate on its own.

- It has been tested with VDXR and partially tested with SteamVR OpenXR.
- Both FidelityFX and NVIDIA Optical Flow can provide around a 40% frame-rate increase, based on current testing.
- NVIDIA Optical Flow generally produces a cleaner image, while FidelityFX usually shows more artifacts.
- Results vary by GPU, scene, resolution and headset refresh rate.
- Some overlays, including xrFPS, may report half the perceived frame rate while OFXR is active.
- The game must be started through `Witcher3VRLauncher.exe` for OFXR to load.

### ReShade, DLSS5 and OptiScaler

Choose the integration you want directly from the launcher:

- **ReShade support**, on its own or alongside OptiScaler.
- **OptiScaler support**, using our stereo-compatible DLSS5/VR fork.

The bundled ReShade is [our unofficial ReShade VR fork](https://github.com/tig3rmast3r/ReShade_VR), based on ReShade 6.8.0. It includes a targeted fix for a startup crash when SteamVR uses DirectX 11 and 12 together. This fix has been validated with Witcher3VR; it is not a claim of compatibility with every game or VR setup.

Disable DLSS Override before using OptiScaler. Its neural-rendering route requires a compatible NVIDIA NR DLL supplied separately; that file is not included with the mod.

### Cinema Modes

Cinema mode supports `5:4`, `4:3`, `16:10` and `16:9` presentation formats.

### First-Person View

First-person mode is available for exploration, combat, horseback riding and sailing. Some animations and unusual camera situations can still look better in third person.

### VR HUD Editor

HUD elements can be positioned in VR and saved from inside the game. Separate presets can be kept for different play styles and presentation modes.

## Current Support

| Feature | Status |
| --- | --- |
| OpenXR | Supported |
| VDXR | Supported and recommended for Quest headsets |
| SteamVR OpenXR | Supported; standard Presentation Size |
| Mouse and keyboard / gamepad | Supported |
| VR motion controllers | Not supported |
| AER + AFW | Supported |
| Stereo rendering | Supported |
| Mono rendering | Preliminary |
| No AA / FXAA / TAAU / DLSS / DLAA | Supported, depending on render mode |
| OptiScaler | Supported |
| ReShade | Supported through our fork with the validated SteamVR startup crash fix |
| DLSS5 via OptiScaler | Supported through the stereo-compatible fork |
| OFXR frame generation | Experimental; tested with VDXR and partially with SteamVR OpenXR |

The most frequently tested configurations are Quest 3 through VDXR and Pimax headsets through SteamVR OpenXR. Other OpenXR runtimes may work but need more user feedback.

The ForceDLAA parameter-query and resolution-override approach is adapted from
[DLSSTweaks](https://github.com/emoose/DLSSTweaks) by emoose.

### Not Supported

- Ray tracing
- Screen Space Reflections (High)
- Far/Distant game camera modes for exploration, combat and horse riding; only close/near cameras are currently corrected

## Requirements

- The Witcher 3 Next-Gen (2022), v4.0.4, DirectX 12 version
- A working OpenXR runtime for your headset
- A mouse and keyboard or gamepad
- Windows 10 or Windows 11
- A VR-ready GPU and CPU
- The current Microsoft Visual C++ Redistributable

DLSS and DLAA require a compatible NVIDIA GPU. OptiScaler and OFXR have their own hardware and software requirements.

## Installation

**Everything needed for the launcher-managed integrations is already included except the NVIDIA NR DLL:** ReShade, the custom VR OptiScaler, OFXR and the launcher are supplied in the package. ReShade add-ons are optional and user-managed.

1. Extract the release archive into the Witcher 3 game folder, the directory containing `bin`, `content` and `mods`.
2. Merge the folders and overwrite the package files when prompted.
3. To use OptiScaler, obtain a compatible `nvngx_dlssnr.dll` for your GPU (**RTX 50xx or RTX 40xx**) separately and put it in `The Witcher 3\bin\x64_dx12\witcher3vr-dlss5-reference`. This folder is empty in the release. ReShade alone does not need the NR DLL.
4. Run `bin\x64_dx12\Witcher3VRLauncher.exe`.
5. Choose Off, OptiScaler, ReShade or OptiScaler + ReShade, then select **Save & Launch**. OptiScaler needs a DLSS/DLAA rendering mode. The launcher handles its own runtime files; choose **Off** and save to disable the integrations. ReShade add-ons, if desired, must be installed and configured separately by the user.

The package layout includes the VR DLL, launcher, OFXR, ReShade, one OptiScaler reference, configuration, scripts and bundled mod files inside the correct `bin\x64_dx12`, `mods`, `dlc` and `Witcher3VR` folders. The launcher copies the selected integration from its reference folder into the DX12 directory; it never modifies the references. The NVIDIA NR DLL is not bundled. The root `OptiScaler.ini` stays in place and retains user settings. Install the whole package when changing release versions; replacing only the DLL can leave incompatible files behind.

The separate **RenderDoc Addon** is only for diagnostic captures. It is not required to play.

Back up any files you have edited manually before installing a new release.

## Launcher Guide

### First-Time Setup

Press **Configure Settings for VR** at least once before playing. It applies the tested game profile and creates a backup that can be restored later with **Restore Original Settings**.

### Resolution

`AUTO` follows the headset and runtime configuration. Use a fixed resolution when you want predictable image quality or need to reduce GPU load.

Lower resolutions improve performance but reduce clarity. Very high resolutions can become GPU-limited even when the game itself appears to have spare performance.

### Presentation Size

Presentation Size now preserves the complete image instead of cropping it. Lowering the slider presents the same rendered pixel resolution in a smaller area of your view, increasing visible pixel density and improving perceived clarity without raising the render resolution.

`1.00` keeps the normal presentation unchanged. Lower values can leave unused space around the image; choose a value that is comfortable for your headset.

**Alt resize is no longer needed and has been removed.** The standard slider works with both VDXR and SteamVR.

### World Detail Range

The default value is `100%`, preserving the normal extended world-detail range.

Lowering it can recover performance in cities and other CPU-heavy areas by reducing how far detailed objects remain visible. At `40%`, the extended LOD/culling correction is effectively disabled. Use the highest value that gives stable performance on your headset.

### Asymmetric Projection

Asymmetric projection is enabled by default and provides the intended image and performance path. Press `F2` in game for a temporary switch to symmetric projection when diagnosing a visual problem.

### OFXR Frame Generation

- **FidelityFX:** frame generation with more visible artifacts than NVIDIA Optical Flow.
- **Nvidia:** legacy NVIDIA Optical Flow presentation. Medium with 50% optical-flow input resolution is a useful starting point; final output remains full resolution.
- **Nvidia (new):** experimental OFXR 0.3.0 NVIDIA presentation, available only with OptiScaler. Try it if legacy motion is uneven below the headset refresh rate.
- **FSR 3.1:** requires OptiScaler and a DLSS/DLAA rendering mode; uses the game's depth/motion inputs and separate eye histories.
- **Off:** native game frames only.

Start the game from the VR Launcher whenever OFXR is enabled.

The separate OFXR DLL is v0.2.1. With OptiScaler enabled it bootstraps OpenXR
for the embedded implementation; no tray application is needed. The launcher
saves settings in both `ofxr_bridge.ini` and the persistent root `OptiScaler.ini`.
OptiScaler's **OFXR VR Framegen** GUI can change them live, even with Neural
Rendering switched off.

Separate OFXR and OptiScaler logging checkboxes are in the launcher. Logging,
RAM timing capture, cadence diagnostics and the purple synthetic marker are off
by default. The optional purple rectangle helps check whether generated frames
reach the headset; FPS counters alone do not guarantee headset scanout.

### ReShade and OptiScaler

The integration dropdown offers Off, OptiScaler, ReShade and OptiScaler + ReShade, in that order. The default is Off.

ReShade is installed as a plain runtime. Its add-ons and their configuration are entirely user-managed: the launcher does not copy, modify or remove them. Existing unrelated add-ons and ReShade settings are preserved. OptiScaler settings use Delete; ReShade uses F4. AFW debug uses Ctrl+F6.

Disable DLSS Override before enabling OptiScaler. OptiScaler selections need a DLSS/DLAA render route. The INI `config_version` is a settings-format migration marker, not a required mod build version; a different value does not block launch. The experimental controller-locked HUD option holds the gameplay HUD in the F9-recentered front direction instead of following headset turns.

### Cinema

Choose `5:4`, `4:3`, `16:10` or `16:9` according to the content and the amount of peripheral view you want.

### Diagnostic Logging

Enable **Diagnostic Logging** only while collecting detailed information for a bug report. It is heavy and can affect timing or performance measurements.

Quick XR/HUD logs and optional route/pipeline recorders also require Diagnostic
Logging. With the switches off, these writers and recorders stay inactive.

## Recommended Game Settings

These are starting points rather than strict requirements:

| Setting | Recommendation |
| --- | --- |
| DirectX | DX12 |
| Ray tracing | Off |
| Screen-space reflections | Off or Low |
| NVIDIA Reflex | Off |
| Motion blur | Off |
| Blur | Off |
| Bloom | Off if it causes discomfort or visual artifacts |
| Lens effects | Off if they look detached in VR |
| Shadows | High or lower when CPU-limited |
| HairWorks | Off |
| VSync | Off |
| Frame-rate limit | Off, or set to suit the headset and frame-generation mode |

### Texture Quality and LOD

The Texture Quality setting also affects LOD behaviour. Higher values keep detailed geometry and textures visible farther away, but may increase distant shimmering.

Suggested values:

- **No AA / FXAA:** Medium
- **TAAU / DLSS:** Medium or High
- **DLAA:** Higher values may be practical if performance allows

## Useful Shortcuts

| Key | Action |
| --- | --- |
| `F2` | Toggle asymmetric/symmetric projection temporarily |
| `F3` | Write the recent Route Log and Performance Log events |
| `F7` | Switch between the VR and Cinema3D HUD layouts |
| `F8` | Toggle Standard and Near views |
| `F9` | Recenter the VR view (configurable: `[openxr] recenter_key`, e.g. `F5`) |
| `F10` | Toggle Cinema Mode |
| `F11` | Toggle First Person |

Shortcuts may conflict with overlays or other mods. Disable or rebind conflicting software when a key does not respond.

## HUD Editor

| Control | Action |
| --- | --- |
| `Insert` | Open the editor, or save and close it |
| `Q` / `E` | Select the previous or next panel |
| Arrow keys | Move the selected panel |
| Mouse wheel | Resize the selected panel |
| `R` | Reset the selected panel |
| `X` | Reset the active HUD layout |
| `F7` | Switch between the VR and Cinema3D HUD layouts |

The editor can separately position gameplay and cutscene subtitles, dialogue text and choices. Keep separate backups of layouts you want to reuse after reinstalling the mod.

## Known Issues

- Intermittent crashes, hangs and black-screen starts when using OFXR and integrations remain under investigation, including with ReShade/AER. The bundled ReShade fork fixes one reproduced SteamVR startup crash, not every failure. If affected, try disabling OFXR and selecting **Off** in the integration dropdown, and report your exact settings and diagnostic logs.
- OFXR has been tested with VDXR and partially with SteamVR OpenXR. Other runtimes need validation.
- Some FPS overlays report half frame rate while OFXR is active.
- Mono mode is preliminary. Some cutscenes may not retain DLSS/DLAA or TAAU.
- Ray tracing is temporarily unsupported because it does not currently work correctly with asymmetric projection.
- Screen-space reflections on High can produce distracting stereo artifacts.
- Motion blur, bloom and lens effects can look uncomfortable or detached in VR.
- Some terrain can still look incorrect in a few Full VR cutscenes.
- First-person mode can expose animation and camera issues, especially while moving backward or during scripted sequences.
- Very distant cameras and some cutscenes may still show visual inconsistencies.

You are free to use other mods, but third-party mod compatibility is not
supported during this stage of development. Before reporting a Witcher 3 VR
bug, disable all other mods and reproduce the problem on an otherwise supported
installation. Please do not open issues for problems that occur only while
another mod is installed. This is a temporary development-scope limitation, not
a restriction on using mods.

## Troubleshooting

### The game crashes when ReShade is enabled

- Select **Off** in the launcher's integration dropdown and save.
- Use the ReShade build supplied with this package: it includes the validated SteamVR startup crash fix. An unmodified ReShade installation does not include our change. Other crashes may have a different cause.
- When reporting it, include your selected integration, GPU/driver, headset/runtime, and the ReShade and Witcher3VR logs.

### The game does not enter VR

- Confirm that the correct OpenXR runtime is active.
- Turn off Windows HDR.
- Close RivaTuner Statistics Server (RTSS), including any overlay using it.
- Start the game through `bin\x64_dx12\Witcher3VRLauncher.exe`.
- Temporarily disable overlays and other DLL injectors.
- Reinstall the complete release package instead of replacing only the DLL.

### The image is distorted after changing Presentation Size

- Restore Presentation Size to `1.00`.

### Performance is poor in cities

The Witcher 3 is already CPU-bound in busy cities, and the VR mod increases CPU load further. A recent high-end CPU is required for consistently high frame rates in these areas.

- Lower World Detail Range gradually.
- Reduce shadows, crowd density and background characters.
- Try AER + AFW instead of full stereo.
- Reduce render resolution before reducing texture quality.
- Test OFXR with VDXR if native performance is still insufficient.

### OptiScaler does not work

- Disable DLSS Override.

## Bug Reports

When reporting a problem, include:

- Headset and OpenXR runtime
- GPU and driver version
- Render mode and anti-aliasing/upscaler mode
- Resolution, Presentation Size and World Detail Range settings
- Whether OFXR or OptiScaler is active
- A short description of where the issue occurs
- A diagnostic log only when requested or when it clearly captures the problem

## Support

The mod is free and publicly available. Donations are entirely optional and
never provide exclusive builds, features, or support.

If you would like to support development, you can do so through
[Ko-fi](https://ko-fi.com/tig3rmast3r) or by using the Sponsor button at the
top of the repository.

## Credits

### ReShade, DLSS5 and upscaling

- [Patrick Mours (crosire) and ReShade contributors](https://github.com/crosire/reshade) — post-processing runtime, overlay and add-on API. The package uses [our unofficial ReShade VR fork](https://github.com/tig3rmast3r/ReShade_VR), based on 6.8.0, with a targeted DirectX 11/12 compatibility fix for the reproduced SteamVR startup crash. Original copyright and license notices are retained.
- [The OptiScaler team and contributors](https://github.com/optiscaler/OptiScaler) — the original upscaling integration.
- [Dagherbou / OptiScaler_DLSSNR](https://github.com/Dagherbou/OptiScaler_DLSSNR) — DLSS Neural Rendering support in OptiScaler. [Our VR fork](https://github.com/tig3rmast3r/OptiScaler_DLSSNR_VR) adds separate processing for each eye to prevent flickering and unstable colours.
- [sadbee166](https://github.com/sadbee166) — author of [OptiScaler_DLSSNR PR #6](https://github.com/Dagherbou/OptiScaler_DLSSNR/pull/6), which introduced running DLSS Neural Rendering before DLSS upscaling and is the basis of the VR v2 pre-DLSS path.
- [NVIDIA / Streamline and NGX](https://github.com/NVIDIA-RTX/Streamline) — DLSS technologies and integration interfaces. NVIDIA DLSS5 runtime DLLs are not distributed with Witcher3VR.
- [AMD / FidelityFX](https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK) — upscaling and optical-flow technology used by the integrations.
- [Omar Cornut and contributors / Dear ImGui](https://github.com/ocornut/imgui) — interface components used by the overlays and add-ons.

### VR, frame generation and other components

- [PureDark](https://github.com/PureDark) — the AFW component used by AER + AFW.
- [OFXR Bridge](https://github.com/tig3rmast3r/OFXR-Bridge) — optical-flow-based VR frame generation, with AMD and NVIDIA backends.
- [praydog / REFramework](https://github.com/praydog/REFramework) — DX12
  hooking and VR architecture reference
- [praydog / UEVR](https://github.com/praydog/UEVR) — additional VR
  implementation reference
- [emoose / DLSSTweaks](https://github.com/emoose/DLSSTweaks) — ForceDLAA
  parameter-query and resolution-override approach
- [Next Gen Movement Input Lag Fix — Fumio Edition](https://www.nexusmods.com/witcher3/mods/7586)
  — behavior-graph foundation for the optional Fast Transitions DLC
- [MinHook](https://github.com/TsudaKageyu/minhook) — Windows API hooking
  library
- [Khronos OpenXR SDK](https://github.com/KhronosGroup/OpenXR-SDK)
- [Microsoft DirectX-Headers](https://github.com/microsoft/DirectX-Headers)
- [baldurk and contributors / RenderDoc](https://github.com/baldurk/renderdoc) — graphics diagnostics and capture tooling.
- Testers who provided headset-specific feedback, logs and performance comparisons

See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md) for copyright notices and
third-party licensing information.

## Contributing

Development information is available in [CONTRIBUTING.md](CONTRIBUTING.md). The README intentionally focuses on installing, configuring and playing the mod.

## License

Original Witcher 3 VR code is released under the [MIT License](LICENSE), unless
otherwise stated.

Third-party software remains the property of its respective authors and is
distributed under its own license terms. The project's MIT license does not
relicense third-party components or NVIDIA SDK materials.

## Disclaimer

> This is an unofficial fan work and is not approved or endorsed by
> CD PROJEKT RED.

This project is not affiliated with CD PROJEKT RED, CD PROJEKT S.A., NVIDIA, or
the authors of the third-party projects listed above.

All trademarks, game content, and related intellectual property belong to
their respective owners. This project does not distribute game assets and
requires a legally obtained copy of *The Witcher 3: Wild Hunt*.

The software is provided as-is and without warranty. Use it at your own risk
and keep backups of saves and configuration files.

This project is intended to comply with the
[CD PROJEKT RED Fan Content Guidelines](https://www.cdprojektred.com/en/fan-content).
