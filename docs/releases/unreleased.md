# Unreleased changes

Collected on `remaster` since v0.9.8v2-remastered.4; this file becomes the next release's notes
(`v0.9.8v2-remastered.<n>.md`) when a release is made.

## Changes compared to v0.9.8v2-remastered.4

- **Controller-locked HUD with DLSS per eye (5.x).** With the game's DLSS the HUD stayed glued to
  the headset although `hud_controller_locked=1`: the mod could not tell which eye a captured HUD
  image belonged to and kept the game's own, head-locked HUD. It now matches them by submission
  order with DLSS per eye (#23). INI override: `[engine] remaster_submitted_hud_join`.

- **Docs: recommended setup on 5.x.** The README now recommends Windows with a Quest or Pico over
  Virtual Desktop (symmetric field of view, which the 5.x port renders without cropping) and
  explains the performance tip of reducing the FOV tangent in the Virtual Desktop Streamer settings
  on the PC.
- **System menu at full frame rate and with anti-aliasing (5.x):** the double image behind the
  system menu had a simple cause: the game renders one view behind a menu, but the mod still moved
  the camera by half the eye distance, alternating every frame. That shift is now skipped while a
  menu is open, and FSR/DLSS keep one history for the menu view. The workaround of .4 (every
  second frame held) is removed, so the scene behind the menu runs at the full frame rate and is
  anti-aliased again. Taken over from kaarele's fork (`MENU-MONO-CAMERA`), plus the FSR part.
- **Sun shadows once per eye pair (5.x):** the game computed the sun shadow cascades for each eye
  separately, although both eyes of a pair see the same sun shadows. The second eye now reuses the
  first eye's shadow map (`[engine] remaster_shadow_pair_reuse=1`, default; `0` turns it off). It
  checks that nothing changed in between and switches itself off after repeated mismatches. Saves
  about 3.5 ms of shadow work per eye pair in the test scene. Taken over from kaarele's fork.
