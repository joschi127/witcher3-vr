# Remaster port: open issues

Current list of known problems and untested areas on 5.01. Keep it short and current: remove an
entry when it is fixed (and record the fix in `remaster-port-status.md`, the chronological log),
update an entry when something new is learned.

Test setup unless noted: Pico 4 over Virtual Desktop (VDXR), symmetric projection
(`native_stereo=0`), 1748x1468 per eye, FXAA, No-AA route, `AllowShafts=false`.

Requirement on AMD GPUs: Adrenalin 26.9.2 or newer. Older drivers render some 5.x grass solid
black, with or without the mod (see `remaster-port-status.md`, 2026-10-17).

## Bugs

### Cutscene background trees follow the head (critical)
- Seen in the first Ciri cutscene and in at least one other cutscene (2026-10-15): the cutscene
  itself and its camera are fine, but the trees/forest in the background do not stay in place. They
  move with the head like a HUD element, so they end up completely displaced.
- Not seen in gameplay.
- Suspect: the background is drawn with a camera or view that the mod does not update with the HMD
  pose on 5.01 (for example a cutscene-specific background layer, distant-forest LOD or impostor
  pass with its own view constants). Not yet checked on 4.04 at the same scene.
- Strong candidate: the foliage native snapshot (V1521), which is not working on 5.01. It covers
  exactly two shader pairs, distant trees and near fronds, and copies their per-draw camera
  constants (VS b0, and b12 with the previous-frame camera) into private buffers so that each eye's
  draw keeps its own values. On 5.01 it falls back on every draw (`reason=graphics_state`: b0 is a
  root CBV there, b12 sits in another table), so these draws read the game's transient constants,
  which may hold another eye's or a non-HMD camera. Open: why it shows in cutscenes but not in
  gameplay.
- Also possible: a 4.04 problem as well. Upstream removed its foliage orientation correction in
  V1519/V1521 ("incomplete ... a complete correction can be reintroduced later"); the snapshot
  that remains copies the constants unchanged. Upstream's README lists "Some terrain can still
  look incorrect in a few Full VR cutscenes" as a known issue. Next step: the same cutscene on 4.04
  in the headset.

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
