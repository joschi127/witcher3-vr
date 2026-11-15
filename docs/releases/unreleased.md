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
- **DLSS per eye in the launcher (5.x, NVIDIA):** the "FSR per eye (5.x)" checkbox is now an
  anti-aliasing choice next to Integration: **FXAA (5.x)**, **FSR per eye (5.x)** or **DLSS per eye
  (5.x)**. DLSS per eye writes the game's DLSS (`AAMode=6`, `AllowDLSS=true`) and
  `[engine] temporal_backend=dlss`, which before had to be set by hand. Confirmed working on an
  NVIDIA RTX 5090 by a user (#23); not testable on the AMD test machine of this fork. The Linux tool
  accepts `configure --aa dlss` on 5.01 as well.
- **No flash at cutscene camera cuts with FSR or DLSS per eye (5.x):** the game never restarts the
  upscaler history at a camera cut, so after a cut in a dialogue or cutscene both eyes briefly
  blended in the previous shot. The mod now restarts both eye histories at every Full VR camera
  cut (`[engine] remaster_cut_history_reset`, on by default), as it already did for TAAU on 4.04.
  Tested in the headset on 5.01. For test runs, `[debug] history_event_log=1` writes only these
  history events to `witcher3vr.log`.
- **No left-eye glitch in dialogues and no double pause menu on 5.x:** `[openxr] native_stereo=1`
  half-enabled the 4.04 asymmetric projection, which is not ported to 5.x. In Full VR dialogues the
  left eye then briefly showed the right eye's image now and then, and the pause menu showed a
  double image. Merely opening the launcher set it to 1 until the next Save. The mod now ignores
  `native_stereo` (and the F2 toggle) on 5.01, and the launcher keeps it at 0 on 5.x. Tested in the
  headset on 5.01 (flicker in 4 of 4 dialogue runs before, 0 of 3 after).
- **Loading screen stays in place on 5.x:** the main menu and the loading screen after Continue
  followed every head turn (they were deliberately head-locked during startup), which felt jerky
  with the loading screen's moving picture. They are now fixed in the room like the in-game menus,
  in the direction you look when they appear; if the headset moves more than 0.5 m (for example
  while putting it on), they are placed again. Tested in the headset on 5.01. 4.04 is unchanged.
- **No double image behind the system menu on 5.x:** behind the system menu the game keeps
  rendering both eyes, and the menu panel alternated between them from frame to frame, so the
  scene behind the menu flickered as a double image (with FXAA and FSR alike). The panel now shows
  one eye. The background runs at half the frame rate there, which is not noticeable behind a menu.
  Tested in the headset on 5.01.
