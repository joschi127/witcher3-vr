# Remaster port: open issues

Current list of known problems and untested areas on 5.01. Keep it short and current: remove an
entry when it is fixed (and record the fix in `remaster-port-status.md`, the chronological log),
update an entry when something new is learned.

Test setup unless noted: Pico 4 over Virtual Desktop (VDXR), symmetric projection
(`native_stereo=0`), 1748x1468 per eye, FXAA, No-AA route, `AllowShafts=false`.

## Bugs

### Cutscene background trees follow the head (critical)
- Seen in the first Ciri cutscene and in at least one other cutscene (2026-10-15): the cutscene
  itself and its camera are fine, but the trees/forest in the background do not stay in place. They
  move with the head like a HUD element, so they end up completely displaced.
- Not seen in gameplay.
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

### One kind of grass is always very dark, almost black
- Seen in gameplay in the Kaer Morhen courtyard and at other places (2026-10-15). The same kind of
  grass is always black, never right; other vegetation looks normal. A 5.01 save at the courtyard
  exists locally.
- Likely related to the open foliage port: the foliage native snapshot (V1521) falls back on every
  draw on 5.01 (`reason=graphics_state`, it expects the 4.04 VS bindings). Not yet checked flat or
  on 4.04 at the same spot.

## Usability

### Gameplay HUD is head-locked and close to the edges of the view
- The HUD works, but it follows the head and its elements sit near the borders of the field of view,
  which makes them hard to read (2026-10-15). Head-locked is the upstream design (also on 4.04;
  `hud_controller_locked=1` is an experimental alternative). Options to try: `[openxr] hud_size`
  (0.5-1.5), `hud_stereo_shift_px` (perceived depth), the in-game HUD editor (`Insert`, needs the
  editor script; untested on 5.01).

## Untested in the headset
- Combat, horse, crossbow and the open world remain untested. The session of 2026-10-15 at 90 Hz
  with diagnostics off was "nice and playable" overall.

## Not ported yet
- Foliage native snapshot (V1521) on 5.01.
- Native asymmetric projection (symmetric runtimes only so far).
- TAAU/DLSS/RT backends (5.01 forces `temporal_backend=none`, No-AA route).
- Build-aware launcher "Configure Settings for VR" (list of 5.x settings in `guidelines.md`).

## Verified in the headset (for reference)
- Third-person gameplay with correct 3D depth in both eyes, intro, menus, loading screens, HUD,
  fire/smoke (2026-10-13).
- Cutscenes, including the cinema camera position (2026-10-15).
