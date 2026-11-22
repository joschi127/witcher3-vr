# Unreleased changes

Collected on `remaster` since v0.9.8v2-remastered.5; this file becomes the next release's notes
(`v0.9.8v2-remastered.<n>.md`) when a release is made.

## Changes compared to v0.9.8v2-remastered.5

- 5.01: the controller-locked HUD (`hud_controller_locked=1`) now also works on systems where the
  game records the HUD and the final image on different GPU queues with FSR or FXAA (seen on NVIDIA
  with SteamVR; #23). The submission-order HUD matching (`[engine] remaster_submitted_hud_join`) is
  now on by default on 5.01, no INI change needed. While it is on, `[openxr] steady_icons` (launcher:
  "Steady Icons") is ignored on 5.01, because it kept the matching from starting.
- 5.01: the log now names the window and module of every swapchain the game creates, and
  `[engine] remaster_skip_zero_size_swapchain=1` (default off) leaves empty 0x0 swapchains of other
  windows alone, for diagnosing setups where such a swapchain appears before the game's own.
- The launcher has its own icon (a VR headset with glowing yellow eyes) in Explorer, the taskbar
  and its window, instead of the generic Windows application icon.

