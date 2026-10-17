param(
    # Release name of this fork, e.g. v0.9.8v2-remastered.1
    [Parameter(Mandatory)]
    [string] $Version,
    # Markdown release notes copied into the package as RELEASE_NOTES.md
    [Parameter(Mandatory)]
    [string] $ReleaseNotes,
    # Original Witcher 3 VR release the third-party runtime files come from.
    [string] $UpstreamRepository = 'tig3rmast3r/witcher3-vr',
    [string] $UpstreamTag = 'v0.9.8v2',
    [string] $UpstreamAsset = 'Witcher3VR-v0.9.8v2-V1581-Main.zip',
    [string] $UpstreamSha256 = 'a59f96f43db9c9509d0ee07638a7f5d1edd280010a836105b6e93f820b31bd5a',
    [string] $CacheDirectory = 'local/upstream-release',
    [string] $BuildDirectory = 'build/release-vs2026',
    [string] $OutputDirectory = 'local/releases',
    [switch] $AllowDirtyWorktree
)

# Builds this fork's release package from the original release plus this
# repository's build:
# - third-party runtime files (OptiScaler, OFXR, ReShade, OpenXR loader, game
#   scripts) are taken unchanged from the pinned original release;
# - PureDark's PDAFWPlugin.dll and the Fast Transitions DLC are left out (not
#   redistributed by this fork, see THIRD_PARTY_NOTICES.md);
# - dxgi.dll, the launcher, the documents and the example INI come from here.

$ErrorActionPreference = 'Stop'
$repositoryRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repositoryRoot

function Resolve-RepoPath([string] $path) {
    if ([System.IO.Path]::IsPathRooted($path)) { return $path }
    return Join-Path $repositoryRoot $path
}

$commit = (git rev-parse HEAD).Trim()
$dirty = git status --porcelain --untracked-files=no
if ($dirty -and -not $AllowDirtyWorktree) {
    throw "The worktree has uncommitted changes; commit them or pass -AllowDirtyWorktree."
}

$dll = Resolve-RepoPath "$BuildDirectory/Release/dxgi.dll"
$launcher = Resolve-RepoPath "$BuildDirectory/launcher/Release/Witcher3VRLauncher.exe"
foreach ($file in @($dll, $launcher, (Resolve-RepoPath $ReleaseNotes))) {
    if (-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Missing $file" }
}

# Original release archive, downloaded once and verified.
$cache = Resolve-RepoPath $CacheDirectory
New-Item -ItemType Directory -Force -Path $cache | Out-Null
$upstreamZip = Join-Path $cache $UpstreamAsset
if (-not (Test-Path -LiteralPath $upstreamZip -PathType Leaf)) {
    gh release download $UpstreamTag -R $UpstreamRepository -p $UpstreamAsset -D $cache
    if ($LASTEXITCODE -ne 0) { throw "Downloading $UpstreamAsset failed" }
}
$actualSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $upstreamZip).Hash.ToLowerInvariant()
if ($actualSha -ne $UpstreamSha256.ToLowerInvariant()) {
    throw "SHA256 mismatch for $UpstreamAsset`: $actualSha"
}

$output = Resolve-RepoPath $OutputDirectory
$staging = Join-Path $output "staging-$Version"
if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
New-Item -ItemType Directory -Force -Path $staging | Out-Null
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::ExtractToDirectory($upstreamZip, $staging)

# Not redistributed by this fork.
foreach ($relative in @('bin/x64_dx12/PDAFWPlugin.dll', 'dlc/dlcmovementinputfix', 'Witcher3VR/BUILD.txt')) {
    $path = Join-Path $staging $relative
    if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force }
}
$dlcRoot = Join-Path $staging 'dlc'
if ((Test-Path -LiteralPath $dlcRoot) -and -not (Get-ChildItem -LiteralPath $dlcRoot)) {
    Remove-Item -LiteralPath $dlcRoot -Force
}

# This fork's files.
Copy-Item -LiteralPath $dll -Destination (Join-Path $staging 'bin/x64_dx12/witcher3vr-mod-reference/dxgi.dll') -Force
Copy-Item -LiteralPath $launcher -Destination (Join-Path $staging 'bin/x64_dx12/Witcher3VRLauncher.exe') -Force
$documentation = Join-Path $staging 'Witcher3VR'
foreach ($name in @('README.md', 'LICENSE', 'THIRD_PARTY_NOTICES.md')) {
    Copy-Item -LiteralPath (Resolve-RepoPath $name) -Destination $documentation -Force
}
Copy-Item -LiteralPath (Resolve-RepoPath $ReleaseNotes) -Destination (Join-Path $documentation 'RELEASE_NOTES.md') -Force
Copy-Item -LiteralPath (Resolve-RepoPath 'witcher3vr.example.ini') -Destination (Join-Path $documentation 'config/witcher3vr.example.ini') -Force

$dllSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $dll).Hash
$launcherSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $launcher).Hash
@(
    "Version=$Version"
    "Commit=$commit"
    "Worktree=$(if ($dirty) { 'dirty' } else { 'clean' })"
    "Base=$UpstreamRepository $UpstreamTag ($UpstreamAsset)"
    "dll.sha256=$dllSha"
    "launcher.sha256=$launcherSha"
    ''
    'Fork of Witcher 3 VR by tig3rmast3r with The Witcher 3 Remastered (5.x) support.'
    'Not included: PureDark AFW (PDAFWPlugin.dll, for AER + AFW) and the Fast Transitions DLC;'
    'both are part of the original release: https://github.com/tig3rmast3r/witcher3-vr/releases'
    'DLSS5 files are not bundled. Put nvngx_dlssnr.dll from your separately obtained package in bin/x64_dx12/witcher3vr-dlss5-reference.'
) | Set-Content -LiteralPath (Join-Path $documentation 'BUILD.txt') -Encoding UTF8

$zipName = "Witcher3VR-$Version-Main.zip"
$zip = Join-Path $output $zipName
if (Test-Path -LiteralPath $zip) { Remove-Item -LiteralPath $zip -Force }
[System.IO.Compression.ZipFile]::CreateFromDirectory($staging, $zip,
    [System.IO.Compression.CompressionLevel]::Optimal, $false)
$zipSha = (Get-FileHash -Algorithm SHA256 -LiteralPath $zip).Hash.ToLowerInvariant()
"$zipSha  $zipName" | Set-Content -LiteralPath (Join-Path $output "Witcher3VR-$Version-SHA256.txt") -Encoding ASCII
Remove-Item -LiteralPath $staging -Recurse -Force

"package: $zip"
"sha256:  $zipSha"
"commit:  $commit"
