# Unreleased changes

Collected on `remaster` since v0.9.8v2-remastered.4; this file becomes the next release's notes
(`v0.9.8v2-remastered.<n>.md`) when a release is made.

## Changes compared to v0.9.8v2-remastered.4

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
