# Unreleased changes

Collected on `remaster` since v0.9.8v2-remastered.3; this file becomes the next release's notes
(`v0.9.8v2-remastered.<n>.md`) when a release is made.

## Changes compared to v0.9.8v2-remastered.3

- **Subtitles stay in place with the controller-locked HUD:** with **Controller-locked HUD**
  (`[openxr] hud_controller_locked=1`) the HUD already stayed in the recentered direction during
  gameplay, but in Full VR cutscenes and dialogues it still followed the head, and with it the
  subtitles and dialogue choices. The lock now applies there too. The Cinema panel and menus are
  unchanged. Tested in the headset on 5.01.
- **Sharper HUD text on 5.x:** a HUD made smaller with `hud_size` skipped pixels, so text broke up
  and shimmered with head motion. The HUD is now filtered (`[openxr] hud_filtered_sampling`, on by
  default on 5.x, off on 4.04). Taken over from kaarele's fork.
- **No black rectangle during fades on 5.x:** the game draws its fade to black (loading, scene
  changes, fast travel) into the HUD layer, which showed as a black HUD-sized rectangle in VR. The
  whole view now darkens with the fade instead (`[openxr] hud_fade_mode=uniform`, default;
  `remove` drops the fade, `native` keeps the game's rectangle). Taken over from kaarele's fork,
  with `uniform` as the default here because `remove` showed a short flash at the start of
  cutscenes.
