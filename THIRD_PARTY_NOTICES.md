# Third-party notices

Witcher 3 VR contains or is derived from third-party software. The project's
MIT license applies only to original Witcher 3 VR code. Third-party components
remain governed by their respective terms.

The dependency setup script retrieves exact upstream revisions. Dependency
source trees are not committed to this repository and must not be treated as
being relicensed by Witcher 3 VR.

## MIT-licensed components

The following components are incorporated or adapted under the MIT License:

- **DLSSTweaks**
  - Project: https://github.com/emoose/DLSSTweaks
  - Reference commit used for the ForceDLAA approach:
    `1d2fddbe3d1e8f403f795afc17e7db239a83f6a2`
  - Copyright (c) 2023 emoose
  - License: MIT
- **AMD FidelityFX SDK API headers** (`external/fidelityfx-api`: `ffx_api.h`,
  `ffx_api_types.h`, `ffx_upscale.h`, vendored unchanged)
  - Project: https://github.com/GPUOpen-LibrariesAndSDKs/FidelityFX-SDK
  - Pinned revision: `60f4ea81909200d8542eca14dccb2628b763a9a3` (tag `v2.3.0`)
  - Copyright (C) 2026 Advanced Micro Devices, Inc.
  - License: MIT (listed as such in the SDK's `Kits/FidelityFX/docs/license.md`)
  - Only the API declarations are used. The FidelityFX runtime DLLs are the
    game's own and are neither modified nor distributed.
- **Microsoft DirectX-Headers**
  - Project: https://github.com/microsoft/DirectX-Headers
  - Pinned revision: `2c305c16da8a4450db8d7f1e7d8d014c8bc665ee`
  - Copyright (c) Microsoft Corporation
  - License: MIT
- **Khronos OpenXR headers**
  - Project: https://github.com/KhronosGroup/OpenXR-SDK
  - Pinned revision: `5267613edf3d937e3d77556a106a65c2f82b25c6`
  - Copyright 2017-2026 The Khronos Group Inc.
  - Header SPDX license: `Apache-2.0 OR MIT`
  - Witcher 3 VR relies on the MIT option for the generated OpenXR headers.
- **Khronos OpenXR Loader 1.0.22**
  - Project: https://github.com/KhronosGroup/OpenXR-SDK
  - Pinned revision: `458984d7f59d1ae6dc1b597d94b02e4f7132eaba`
  - Copyright 2017-2020 The Khronos Group Inc.
  - Loader SPDX license: `Apache-2.0 OR MIT`
  - The release package includes the 64-bit loader built from this revision
    under the MIT option.

The MIT License text applying to the components above is reproduced here:

> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
>
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM,
> OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

## Development references

[REFramework](https://github.com/praydog/REFramework) and
[UEVR](https://github.com/praydog/UEVR) by praydog were consulted as DX12,
hooking, OpenXR, and VR architecture references during initial development.
Their source trees are not compiled, committed, or distributed as part of
Witcher 3 VR.

Early private builds obtained MinHook through REFramework's dependency tree.
The public dependency setup instead retrieves MinHook directly from its
official upstream repository under the license reproduced below.

## MinHook and Hacker Disassembler Engine

- Project: https://github.com/TsudaKageyu/minhook
- Pinned revision: `98b74f1fc12d00313d91f10450e5b3e0036175e3`

MinHook - The Minimalistic API Hooking Library for x64/x86  
Copyright (C) 2009-2017 Tsuda Kageyu.  
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Portions of this software are Copyright (c) 2008-2009, Vyacheslav Patkov.

### Hacker Disassembler Engine 32 C

Copyright (c) 2008-2009, Vyacheslav Patkov.  
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

### Hacker Disassembler Engine 64 C

Copyright (c) 2008-2009, Vyacheslav Patkov.  
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

## NVIDIA NGX SDK headers

- Source repository: https://github.com/NVIDIA-RTX/Streamline
- Pinned revision: `e8aaa6eaac968711fb62473d4ae8256dde20919b`
- Governing license:
  https://github.com/NVIDIA-RTX/Streamline/blob/main/external/ngx-sdk/license.txt

The dependency setup retrieves NVIDIA NGX SDK interface headers from the
official Streamline repository. NVIDIA SDK materials are not licensed under
the Witcher 3 VR MIT License and must not be redistributed as a standalone
SDK.

Required NVIDIA notice:

> This software contains source code provided by NVIDIA Corporation.

No NVIDIA game DLL or standalone SDK package is distributed by Witcher 3 VR.
Users must rely on the DLSS/NGX runtime supplied with their legally installed
game and compatible NVIDIA driver.

## Bundled OptiScaler integrations

One custom OptiScaler V23244 is bundled under GPL-3.0 with its upstream license.
The old ordinary OptiScaler runtime is not included. The two private AMD
FidelityFX frame-generation helpers retain AMD's MIT license and do not replace
the original game's libraries.

- Source repository: https://github.com/optiscaler/OptiScaler

The DLSS Neural Rendering integration is based on
[Dagherbou / OptiScaler_DLSSNR](https://github.com/Dagherbou/OptiScaler_DLSSNR).
The bundled stereo-compatible sources and NGX forwarder are available from
[OptiScaler_DLSSNR_VR, V23244](https://github.com/tig3rmast3r/OptiScaler_DLSSNR_VR/tree/v23244).
This revision performs neural rendering before DLSS and preserves separate
per-eye history and render-subrect authority.
The pre-DLSS implementation is adapted from
[sadbee166's OptiScaler_DLSSNR PR #6](https://github.com/Dagherbou/OptiScaler_DLSSNR/pull/6),
head commit `914963ffb641943fba8f19f3c8654f0620fc68ac`.
The custom `nvngx.dll_dlssnr.dll` is a forwarding component, not NVIDIA's
`nvngx_dlssnr.dll`. NVIDIA DLSS5 DLLs are not included.

## OFXR Bridge

OFXR Bridge v0.2.1 (V116) is included as a separate, replaceable OpenXR layer DLL under
LGPL-3.0-or-later. No tray application is included or required.

- Corresponding source: https://github.com/tig3rmast3r/OFXR-Bridge/tree/0.2.1
- The OptiScaler-embedded experimental NVIDIA implementation is provided by
  the V23244 source above; the stand-alone layer remains v0.2.1.
- License texts: `licenses/OFXR-LGPL-3.0.txt` and `licenses/GPL-3.0.txt`.
- AMD FidelityFX optical flow is linked under the notice in
  `licenses/AMD-FidelityFX-MIT.txt`.
- NVIDIA Optical Flow interface notices are retained in
  `licenses/NVIDIA-Optical-Flow-Headers.txt`. The driver supplies its runtime;
  no NVIDIA Optical Flow SDK binary is distributed.

## Dear ImGui

Credit: Omar Cornut and contributors. Overlay UI components retain their MIT
license in `licenses/Dear-ImGui-MIT.txt`.
Source: https://github.com/ocornut/imgui

## PureDark AFW

Credit: PureDark for the separately loaded `PDAFWPlugin.dll` used by AER + AFW.
This third-party component is not relicensed under the Witcher3VR MIT license.
Project: https://github.com/PureDark

This fork's release packages do **not** include `PDAFWPlugin.dll`. AER + AFW is
optional; to use it, copy `PDAFWPlugin.dll` from the original Witcher 3 VR
release (https://github.com/tig3rmast3r/witcher3-vr/releases) into
`bin\x64_dx12`.

## Corresponding source for this fork's packages

This fork's release packages take the third-party runtime files unchanged from
the original Witcher 3 VR release. Mirrors of the corresponding sources are kept
in this fork's account:

- OptiScaler_DLSSNR_VR, tag `v23244`: https://github.com/joschi127/OptiScaler_DLSSNR_VR/tree/v23244
- OFXR Bridge, tag `0.2.1`: https://github.com/joschi127/OFXR-Bridge/tree/0.2.1
- ReShade VR, tag `v6.8.0-vr-v1530`: https://github.com/joschi127/ReShade_VR/tree/v6.8.0-vr-v1530

## RenderDoc in-application API and optional runtime

- Source repository: https://github.com/baldurk/renderdoc
- Pinned revision: `e43b7c14d3c37fab664391db94f7586be29e49a0`
- Vendored file: `renderdoc/api/app/renderdoc_app.h`

The RenderDoc API header is distributed under the MIT license included in the
header. The separate optional RenderDoc Addon contains the custom V1273
RenderDoc 1.45 runtime derived from upstream commit
`2fc0bc04cb95499635f63986a55bc6f67849dd9f`. The addon includes the upstream
MIT license and is not required to play Witcher 3 VR.

## ReShade VR

The bundled ReShade runtime is an unofficial build of ReShade 6.8.0 with the
V1530 DirectX 11/12 interoperability correction. Credit for ReShade belongs to
Patrick Mours and the upstream contributors.

- Upstream: https://github.com/crosire/reshade
- Fork: https://github.com/tig3rmast3r/ReShade_VR
- Source tag: `v6.8.0-vr-v1530`
- License: BSD 3-Clause; original terms reproduced below.

Copyright 2014 Patrick Mours. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


## WiVRn (headless test runtime only)

- Project: https://github.com/WiVRn/WiVRn
- Reference: `server/driver/app_pacer.cpp`, WiVRn 26.9
- Copyright (C) 2025 Patrick Nicolas
- License: GPL-3.0-or-later (text in `licenses/GPL-3.0.txt`)

`tools/w3vr-nullxr/w3vr_nullxr.cpp` reproduces WiVRn's application frame pacer,
so headless tests show the same frame-rate steps as a headset. That tool is
therefore licensed under GPL-3.0-or-later. It is built separately from
`dxgi.dll`, is not linked into the mod, and is not included in release
packages. The rest of the project remains under the MIT License.

## CD PROJEKT RED fan content

Witcher 3 VR is an unofficial fan work and is not approved or endorsed by
CD PROJEKT RED and requires a legally obtained copy of The Witcher 3: Wild
Hunt.

The optional Fast Transitions DLC contains a modified Geralt behavior graph.
Its foundation is the **Next Gen Movement Input Lag Fix — Fumio Edition**
(Nexus Mods project 7586); Witcher 3 VR adds narrowly scoped transition changes
for direct run starts and quick reversals. This component and the underlying
REDengine game data are not relicensed under Witcher 3 VR's MIT License.

This fork's release packages do not include the Fast Transitions DLC (its
redistribution terms are not documented here, and it is based on Next-Gen 4.04
game data). It remains available in the original Witcher 3 VR release.

Fan Content Guidelines:
https://www.cdprojektred.com/en/fan-content
