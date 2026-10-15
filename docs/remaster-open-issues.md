# Remaster port: open issues

Current list of known problems and untested areas on 5.01. Keep it short and current: remove an
entry when it is fixed (and record the fix in `remaster-port-status.md`, the chronological log),
update an entry when something new is learned.

Test setup unless noted: Pico 4 over Virtual Desktop (VDXR), symmetric projection
(`native_stereo=0`), 1748x1468 per eye, FXAA, No-AA route, `AllowShafts=false`.

## Bugs

### Cutscene background trees follow the head (critical, one cutscene so far)
- Seen in the first Ciri cutscene (2026-10-15): the cutscene itself and its camera are fine, but the
  trees/forest in the background do not stay in place. They move with the head like a HUD element,
  so they end up completely displaced.
- Not seen in gameplay, and not in the other cutscenes watched so far.
- Suspect: the background is drawn with a camera or view that the mod does not update with the HMD
  pose on 5.01 (for example a cutscene-specific background layer, distant-forest LOD or impostor
  pass with its own view constants). Not yet checked on 4.04 at the same scene.

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

### Grass very dark, almost black (Kaer Morhen courtyard)
- Seen in gameplay with Ciri in the courtyard (2026-10-15).
- Likely related to the open foliage port: the foliage native snapshot (V1521) falls back on every
  draw on 5.01 (`reason=graphics_state`, it expects the 4.04 VS bindings). Not yet checked flat or
  on 4.04 at the same spot.

## Untested in the headset
- Aiming (crossbow), combat, horse riding, leaving Kaer Morhen (open world, vegetation).
- Comfort and performance at 90 Hz (session of 2026-10-15 ran at 90 Hz with diagnostics off).

## Not ported yet
- Foliage native snapshot (V1521) on 5.01.
- Native asymmetric projection (symmetric runtimes only so far).
- TAAU/DLSS/RT backends (5.01 forces `temporal_backend=none`, No-AA route).
- Build-aware launcher "Configure Settings for VR" (list of 5.x settings in `guidelines.md`).

## Verified in the headset (for reference)
- Third-person gameplay with correct 3D depth in both eyes, intro, menus, loading screens, HUD,
  fire/smoke (2026-10-13).
- Cutscenes, including the cinema camera position (2026-10-15).
