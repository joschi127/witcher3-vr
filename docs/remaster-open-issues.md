# Remaster port: open issues

Current list of known problems and untested areas on 5.01. Keep it short and current: remove an
entry when it is fixed (and record the fix in `remaster-port-status.md`, the chronological log),
update an entry when something new is learned.

Test setup unless noted: Pico 4 over Virtual Desktop (VDXR), symmetric projection
(`native_stereo=0`), 1748x1468 per eye, FXAA, No-AA route, `AllowShafts=false`.

Requirement on AMD GPUs: Adrenalin 26.9.2 or newer. Older drivers render some 5.x grass solid
black, with or without the mod (see `remaster-port-status.md`, 2026-10-17).

## Bugs

### Camera-attached light in cutscenes moves with the head
- Some cutscenes (clearly visible in a dark evening scene at a camp fire, also in a brighter one)
  have a light attached to the camera, pointing where the camera looks (2026-10-17). Flat this is
  invisible; in VR it follows head turns like a head lamp and moves its light spot around.
- Wanted: the light keeps the cutscene camera's own direction (no HMD rotation).
- Cause: REDengine places each camera light from the render camera's position and yaw only
  (`CameraLightPlacement`, called solely by the frame light list that also reads
  `[Rendering] DisableAllCameraLights`); the mod composes the HMD rotation into that yaw.
- Fix in `64b7aad` (pending headset check): the placement is hooked and gets the game camera's
  yaw, recorded wherever the mod composes the HMD rotation (gameplay view rebuild and the Full VR
  reused-camera fallback). Null-HMD check: with the head turned 20 degrees the light keeps the
  camera direction. Camera cuts are unaffected (the light follows each shot's camera as flat).
  Open: headset check; 4.04 regression run (its address 0x718310 has no signature check).

### Distant water switches between blue and white depending on head pose
- River seen from the Kaer Morhen balcony. It shows either blue or white. Holding the head still
  keeps the current state; moving the head switches it. Some head poses show white in the left eye
  and blue in the right (2026-10-15, with light shafts off, so the shafts are ruled out).
- Not reproducible with the static null HMD (only the pose changes it). Does not occur on 4.04 at the
  same spot.
- Suspect: a view-dependent water term (reflection, reflection probe selection, screen-space
  reflection or water fog; 5.01 adds `[PostProcess] WaterFogPerPixel=true`) that switches at a
  threshold of the camera angle or position. Next step: a test script with small camera movements on
  the balcony and a flat check (without the mod) with the same movements.

## Usability

### Gameplay HUD is head-locked and close to the edges of the view
- The HUD works, but it follows the head and its elements sit near the borders of the field of view,
  which makes them hard to read (2026-10-15). Head-locked is the upstream design (also on 4.04;
  `hud_controller_locked=1` is an experimental alternative). Options to try: `[openxr] hud_size`
  (0.5-1.5), `hud_stereo_shift_px` (perceived depth), the in-game HUD editor (`Insert`, needs the
  editor script; untested on 5.01).
- `hud_size=0.8` was clearly better to read than 1.0 (2026-10-15). `hud_size=0.85` with
  `hud_controller_locked=1` (HUD stays in place when turning the head) works well on 5.01 and is
  much better to read (2026-10-16). Kept open until it is a default or a launcher setting.

### Anti-aliasing (FXAA only on 5.01)
- Edges looked rough at Virtual Desktop "High" (1748x1468 per eye). Rendering at the runtime's
  "Ultra" recommendation (1872x1572) was much better (2026-10-16). The render size is fixed in the
  INI; the launcher's `resolution_auto` is not yet usable on 5.01.

## Untested in the headset
- Aiming (crossbow).

## Not ported yet
- Foliage native snapshot (V1521) on 5.01.
- Native asymmetric projection (symmetric runtimes only so far).
- TAAU/DLSS/RT backends (5.01 forces `temporal_backend=none`, No-AA route).
- Build-aware launcher "Configure Settings for VR" (list of 5.x settings in `guidelines.md`).

## Verified in the headset (for reference)
- Third-person gameplay with correct 3D depth in both eyes, intro, menus, loading screens, HUD,
  fire/smoke (2026-10-13).
- Cutscenes, including the cinema camera position (2026-10-15).
- Combat: combat tutorial and the first ghoul fight work well (2026-10-15).
- Horse riding: works, nothing unusual (2026-10-15).
- Grass correct with the AMD driver update; sky gradients smooth in the headset (the banding in
  null-driver eye captures is a capture artefact) (2026-10-17).
- Overall "nice and playable" at 90 Hz with diagnostics off, no stutter (2026-10-15).
