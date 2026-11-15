# Unreleased changes

Collected on `remaster` since v0.9.8v2-remastered.3; this file becomes the next release's notes
(`v0.9.8v2-remastered.<n>.md`) when a release is made.

## Changes compared to v0.9.8v2-remastered.3

- **Subtitles stay in place with the controller-locked HUD:** with **Controller-locked HUD**
  (`[openxr] hud_controller_locked=1`) the HUD already stayed in the recentered direction during
  gameplay, but in Full VR cutscenes and dialogues it still followed the head, and with it the
  subtitles and dialogue choices. The lock now applies there too. The Cinema panel and menus are
  unchanged. Tested in the headset on 5.01.
