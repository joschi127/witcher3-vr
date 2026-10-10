# Witcher 3 Remaster: public information relevant to the port

Collected 2026-10-10 from public sources. Labels: OFFICIAL = CD PROJEKT RED,
PRESS = media, COMMUNITY = GitHub/Nexus/forums. Re-check before relying on details.

## What it is
- Announced 2026-08-25 and released 2026-09-29 as a free update to the same product on PC
  (Steam, GOG, Epic, Battle.net) and consoles. Co-developed by CD PROJEKT RED and Yigsoft
  (also co-developer of REDkit and Phantom Liberty). There is no evidence that the Cyberpunk
  team built it or that Cyberpunk code was backported. The feature set (path tracing, RR,
  multi frame generation) resembles Cyberpunk, but press describes the engine as REDengine 3.
  OFFICIAL: https://press.cdprojektred.com/en/news/1839/the-witcher-3-wild-hunt-remastered-announced-songs-of-the-past-gets-first-look
- Earlier versions stay available "depending on your platform". Classic 1.32 and Next-Gen 4.04
  are on Steam/GOG beta channels, but not on Epic. OFFICIAL FAQ:
  https://support.cdprojektred.com/en/witcher-3/pc/sp-technical/issue/2999/the-witcher-3-wild-hunt-remastered-faq
  (On GOG, the GOTY product 1495134320 still installs 4.04b as a separate product.)
- Patches arrive frequently: 5.00 (09-29), 5.00b/5.00c (10-01), 5.01 (10-08). Expect exe churn,
  so pin the supported exe build.
  https://www.thewitcher.com/us/en/news/52085/patch-5-01-for-the-witcher-3-wild-hunt-remastered-is-live

## Renderer and camera changes (most relevant for the mod)
- "Native DirectX 12 rendering with multithreading". The DX11 exe is gone.
  OFFICIAL: https://www.thewitcher.com/dk/en/news/52041/see-whats-new-in-the-witcher-3-wild-hunt-remastered
- New graphics features:
  - path tracing, RT local shadows, RT HairWorks
  - DLSS 4.5 with Ray Reconstruction, FSR 4 with frame generation, XeSS 2 with frame generation
  - new IBL and tone mapping, reworked SSR and shadows, GTAO
  - new HDR calibration and HD Reworked assets
- **The engine's own TAAU was removed** (PRESS, Wccftech 2026-10-05). Remaining AA options are
  FXAA, DLSS/DLAA, FSR 4 native AA and XeSS native AA. The mod's TAAU route and its TAAU-specific
  hooks/shader fingerprints have no direct counterpart.
  https://wccftech.com/how-to/the-witcher-3-wild-hunt-remastered-pc-performance-analysis-tuning-guide-how-to-get-the-best-experience-on-pc/
- New exploration camera (wider FOV), new dynamic combat camera, changed target-lock switching,
  and photo-mode camera changes (OFFICIAL). Expect changed camera code paths behind the mod's
  camera and stereo hooks.

## Modding
- In-game mod browser (mod.io). REDkit 5.0 adds scope-based script overrides and a precompiled
  script blob. Classic Script Merger is incompatible with the Remaster. Script/XML mods need
  updating (PRESS/COMMUNITY): https://github.com/Nexus-Mods/Vortex/issues/24370
- Native proxy mods (d3d12.dll, dinput8.dll, ReShade's dxgi.dll) are reported to work on the
  Remaster, so the proxy-DLL approach is viable. Collisions between multiple dxgi proxies have
  been reported (COMMUNITY).

## Upstream author's position (COMMUNITY, author's own words)
- #27, 2026-09-08: "i can't say anything until we have the new exe", "most likely it will be
  quite broken". 2026-10-07: "not this month for sure".
  https://github.com/tig3rmast3r/witcher3-vr/issues/27
- #31, 2026-10-07: "it will be something i will take into account when i will port the mod for
  the remaster version". He will not add or change anything else in the Next-Gen version.
  https://github.com/tig3rmast3r/witcher3-vr/issues/31
