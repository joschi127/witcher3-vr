[CmdletBinding()]
param(
    # The game's bin\x64_dx12 directory (contains witcher3.exe, dxgi.dll and witcher3vr.ini).
    [Parameter(Mandatory)]
    [string] $GameDirectory,

    # Input script for the in-game test mode (see src/test_input_script.h).
    # Default: test-input\smoke-continue-remaster.txt next to this script.
    [string] $Script,

    # Optional freshly built dxgi.dll to install before the run. On a launcher layout it is also
    # copied to witcher3vr-mod-reference\, which the launcher uses to recreate the active copy.
    [string] $Dll,

    [string] $RunName = ('smoke-' + (Get-Date -Format 'yyyyMMdd-HHmmss')),

    # Default: build\smoke-runs in the repository.
    [string] $OutputDirectory,

    [ValidateRange(30, 3600)]
    [int] $TimeoutSeconds = 240,

    # Keep the [test] settings in witcher3vr.ini after the run instead of restoring the file.
    [switch] $KeepIni,

    # Extra INI values for this run only, as 'section.key=value' (e.g. 'openxr.enabled=1').
    # Existing keys are replaced in place, missing keys are appended to their section.
    [string[]] $IniSet = @(),

    # OpenXR runtime manifest for the game process only (sets XR_RUNTIME_JSON), e.g. the SteamVR
    # runtime with its null driver for a headset-free OpenXR run. The system runtime is unchanged.
    [string] $OpenXrRuntimeJson
)

# Unattended smoke test: install the DLL and input script, start the game with the test mode,
# wait until it exits, collect logs and captures, and evaluate them. Exit code 0 = passed.
# Never point this at a game build the DLL does not support (see src/game_build.h).

$ErrorActionPreference = 'Stop'
# Windows PowerShell 5.1 leaves $PSScriptRoot empty in parameter defaults under -File.
$scriptsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
if (-not $Script) { $Script = Join-Path $scriptsDirectory 'test-input\smoke-continue-remaster.txt' }
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path (Split-Path -Parent $scriptsDirectory) 'build\smoke-runs'
}

$game = (Resolve-Path -LiteralPath $GameDirectory).Path
$exe = Join-Path $game 'witcher3.exe'
$ini = Join-Path $game 'witcher3vr.ini'
foreach ($required in @($exe, $ini, $Script)) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Not found: $required" }
}
if (Get-Process -Name 'witcher3' -ErrorAction SilentlyContinue) {
    throw 'witcher3.exe is already running.'
}

$runDirectory = Join-Path ([System.IO.Path]::GetFullPath($OutputDirectory)) $RunName
New-Item -ItemType Directory -Force -Path $runDirectory | Out-Null

# Leftovers from earlier runs would be mixed into this run's results.
$staleLogs = @(Get-ChildItem -LiteralPath $game -Filter 'witcher3vr*.log' -File)
$staleCaptures = Join-Path $game 'witcher3vr_captures'
if ($staleLogs.Count -gt 0 -or (Test-Path -LiteralPath $staleCaptures)) {
    $staleDirectory = Join-Path $runDirectory 'stale-before-run'
    New-Item -ItemType Directory -Force -Path $staleDirectory | Out-Null
    $staleLogs | Move-Item -Destination $staleDirectory
    if (Test-Path -LiteralPath $staleCaptures) { Move-Item -LiteralPath $staleCaptures -Destination $staleDirectory }
}

if ($Dll) {
    Copy-Item -LiteralPath $Dll -Destination (Join-Path $game 'dxgi.dll') -Force
    $reference = Join-Path $game 'witcher3vr-mod-reference'
    if (Test-Path -LiteralPath (Join-Path $reference 'dxgi.dll')) {
        Copy-Item -LiteralPath $Dll -Destination (Join-Path $reference 'dxgi.dll') -Force
    }
}
$dllHash = (Get-FileHash -LiteralPath (Join-Path $game 'dxgi.dll') -Algorithm SHA256).Hash

$scriptName = 'witcher3vr-test-' + [System.IO.Path]::GetFileName($Script)
Copy-Item -LiteralPath $Script -Destination (Join-Path $game $scriptName) -Force
$expectedCaptures = 2 * @(Get-Content -LiteralPath $Script |
    Where-Object { $_ -match '^\s*\d+\s+capture\b' }).Count

# Point the [test] section at the script; everything else in the INI stays byte-identical.
$iniBytes = [System.IO.File]::ReadAllBytes($ini)
$iniText = [System.Text.Encoding]::ASCII.GetString($iniBytes)
$newline = if ($iniText.Contains("`r`n")) { "`r`n" } else { "`n" }
$testSection = "[test]${newline}record_input=0${newline}input_script=$scriptName${newline}"
$sectionPattern = '(?ms)^\[test\]\r?\n.*?(?=^\[|\z)'
if ([regex]::IsMatch($iniText, $sectionPattern)) {
    $newIni = [regex]::Replace($iniText, $sectionPattern, { param($m) $testSection })
} else {
    $newIni = $iniText.TrimEnd("`r", "`n") + $newline + $newline + $testSection
}
# powershell.exe -File passes 'a=1','b=2' as one comma-separated string.
$IniSet = @($IniSet | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
foreach ($assignment in $IniSet) {
    if ($assignment -notmatch '^([^.=\s]+)\.([^=\s]+)=(.*)$') { throw "Bad -IniSet value: $assignment" }
    $sectionName = $Matches[1]; $key = $Matches[2]; $value = $Matches[3]
    $section = [regex]::Escape($sectionName)
    $sectionMatch = [regex]::Match($newIni, "(?ms)^\[$section\]\r?\n.*?(?=^\[|\z)")
    if (-not $sectionMatch.Success) {
        # Missing sections are appended, so switches of optional sections can be set too.
        if (-not $newIni.EndsWith($newline)) { $newIni += $newline }
        $newIni += "[$sectionName]$newline$key=$value$newline"
        continue
    }
    $body = $sectionMatch.Value
    $keyPattern = '(?m)^' + [regex]::Escape($key) + '=[^\r\n]*'
    if ([regex]::IsMatch($body, $keyPattern)) {
        $body = [regex]::Replace($body, $keyPattern, { param($m) "$key=$value" })
    } else {
        $trimmed = $body.TrimEnd("`r", "`n")
        $body = $trimmed + $newline + "$key=$value" + $body.Substring($trimmed.Length)
        if ($body -notmatch '\r?\n$') { $body += $newline }
    }
    $newIni = $newIni.Substring(0, $sectionMatch.Index) + $body +
        $newIni.Substring($sectionMatch.Index + $sectionMatch.Length)
}
[System.IO.File]::WriteAllBytes($ini, [System.Text.Encoding]::ASCII.GetBytes($newIni))
if ($OpenXrRuntimeJson) {
    if (-not (Test-Path -LiteralPath $OpenXrRuntimeJson)) { throw "Not found: $OpenXrRuntimeJson" }
    $env:XR_RUNTIME_JSON = (Resolve-Path -LiteralPath $OpenXrRuntimeJson).Path
}

$started = Get-Date
$killed = $false
try {
    $process = Start-Process -FilePath $exe -WorkingDirectory $game -PassThru
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
        $killed = $true
        Stop-Process -Id $process.Id -Force
        $process.WaitForExit(30000) | Out-Null
    }
} finally {
    if (-not $KeepIni) { [System.IO.File]::WriteAllBytes($ini, $iniBytes) }
}
$seconds = [int]((Get-Date) - $started).TotalSeconds

# The log can stay locked for a moment after the process is gone.
Start-Sleep -Seconds 2
Get-ChildItem -LiteralPath $game -Filter 'witcher3vr*.log' -File | Move-Item -Destination $runDirectory -Force
Get-ChildItem -LiteralPath $game -Filter 'witcher3vr-input-recording*.txt' -File |
    Move-Item -Destination $runDirectory -Force
$captures = Join-Path $game 'witcher3vr_captures'
if (Test-Path -LiteralPath $captures) {
    Move-Item -LiteralPath $captures -Destination (Join-Path $runDirectory 'captures') -Force
}

$failures = New-Object System.Collections.Generic.List[string]
$notes = New-Object System.Collections.Generic.List[string]
$log = Join-Path $runDirectory 'witcher3vr.log'
if (-not (Test-Path -LiteralPath $log)) {
    $failures.Add('witcher3vr.log missing (proxy not loaded?)')
    $lines = @()
} else {
    $lines = @(Get-Content -LiteralPath $log)
}

$buildLine = $lines | Where-Object { $_ -match 'Game build ' } | Select-Object -First 1
if ($buildLine) { $notes.Add(($buildLine -replace '^\[[^\]]*\]\s*', '')) }

if ($killed) { $failures.Add("game did not exit within $TimeoutSeconds s (killed)") }
if (-not ($lines | Where-Object { $_ -match 'Test mode script loaded' })) {
    $failures.Add('test mode script not loaded')
}
$written = @($lines | Where-Object { $_ -match 'Test mode capture written' }).Count
if ($written -ne $expectedCaptures) {
    $failures.Add("captures written $written, expected $expectedCaptures")
}
$hookFailures = @($lines | Where-Object {
    $_ -match '\bhook\b.*\bfailed\b|\bfailed\b.*\bhook\b|signature mismatch' })
foreach ($line in ($hookFailures | Select-Object -First 10)) {
    $failures.Add('hook failure: ' + ($line -replace '^\[[^\]]*\]\s*', ''))
}
if ($lines | Where-Object { $_ -match 'Test mode quit' }) {
    $notes.Add('exit: script quit action (fallback)')
} elseif (-not $killed) {
    $notes.Add('exit: game closed itself')
}
if ($lines | Where-Object { $_ -match 'GameInput virtual pad announced' }) {
    $notes.Add('input: virtual GameInput gamepad')
}
if ($lines | Where-Object { $_ -match 'Engine dual render INI auto-start queued' }) {
    $notes.Add('stereo: dual render started')
}

$passed = $failures.Count -eq 0
$summary = @(
    "result: $(if ($passed) { 'PASSED' } else { 'FAILED' })"
    "run: $RunName"
    "script: $([System.IO.Path]::GetFileName($Script))"
    "dxgi.dll sha256: $dllHash"
    "duration: $seconds s"
    "captures: $written/$expectedCaptures"
) + ($IniSet | ForEach-Object { "ini: $_" }) +
    $(if ($OpenXrRuntimeJson) { @("openxr runtime: $([System.IO.Path]::GetFileName($OpenXrRuntimeJson))") } else { @() })
$summary = $summary + ($notes | ForEach-Object { "note: $_" }) + ($failures | ForEach-Object { "failure: $_" })
$summary | Set-Content -LiteralPath (Join-Path $runDirectory 'summary.txt')
$summary | ForEach-Object { Write-Output $_ }
Write-Output "output: $runDirectory"
exit $(if ($passed) { 0 } else { 1 })
