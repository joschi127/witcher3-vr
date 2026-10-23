#!/usr/bin/env python3
"""w3vr-smoke: unattended test runs of the Witcher 3 VR mod on Linux (Proton), like scripts/smoke-test.ps1.

Per run: pre-checks (game closed, CrossProgression=false), backup of dxgi.dll, witcher3vr.ini and the save
folder, optional DLL swap, input script + [test] keys, start through the launcher (Heroic or Steam), wait with
timeout (then kill), collect logs and captures into <output>/<run>/, write summary.txt, restore dxgi.dll and
the INI byte-exact (sha256-checked) and put the saves back the way they were (changed or new save files are
moved into the run folder, never deleted).

dx12user.settings is never restored: the game rewrites it on exit and may refresh the account token in
[Galaxy]. The summary lists changed keys outside [Galaxy] instead.

Exit code 0 = all runs passed. Standard library only; reuses linux/w3vr-linux.py for game discovery.

The default script (scripts/test-input/smoke-continue-remaster-linux.txt) continues the latest save,
walks, turns the camera, opens the game menu and quits through the system menu; it is checked with
--expect-eyes 70000-84000 and --expect-self-exit unless another --script is given. The run starts the real
game: like scripts/smoke-test.ps1 it needs the maintainer's go-ahead.

Usage:
  w3vr-smoke.py [--script FILE] [--dll FILE] [--name NAME] [--repeat N] [--timeout S]
                [--ini-set section.key=value ...] [--note TEXT] [--keep-ini] [--game-dir DIR --prefix PFX]
"""
import argparse
import datetime
import importlib.util
import os
import re
import shutil
import signal
import subprocess
import sys
import time

TOOL_DIR = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.abspath(os.path.join(TOOL_DIR, '..'))
DEFAULT_SCRIPT = os.path.join(REPO, 'scripts', 'test-input', 'smoke-continue-remaster-linux.txt')
# Captures of DEFAULT_SCRIPT between these times are in gameplay and must have both XR eye images.
DEFAULT_EXPECT_EYES = '70000-84000'
DEFAULT_OUTPUT = os.path.join(REPO, 'build', 'smoke-runs')

sys.dont_write_bytecode = True  # no __pycache__ next to the tools
_spec = importlib.util.spec_from_file_location('w3vr_linux', os.path.join(TOOL_DIR, 'w3vr-linux.py'))
w3vr = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(w3vr)

START_TIMEOUT = 180  # seconds until witcher3.exe must appear after the launcher call
# Under Proton there is no GameInputRedist.dll; the game then polls XInput, which the mod drives (lx4/lx5).
EXPECTED_HOOK_FAILURES = ('GameInput GameInputInitialize hook failed',)


def log(msg):
    print('[%s] %s' % (datetime.datetime.now().strftime('%H:%M:%S'), msg), flush=True)


def sha256(path):
    return w3vr.sha256_file(path) if os.path.exists(path) else None


def game_pids():
    """PIDs of witcher3.exe, matched like w3vr-linux.game_running() (Wine's argv[0] with backslashes)."""
    pids = []
    for pid in os.listdir('/proc'):
        if not pid.isdigit():
            continue
        try:
            with open('/proc/%s/cmdline' % pid, 'rb') as f:
                argv0 = f.read().split(b'\0', 1)[0].lower()
        except OSError:
            continue
        if argv0.endswith((b'\\bin\\x64_dx12\\witcher3.exe', b'\\bin\\x64\\witcher3.exe')):
            pids.append(int(pid))
    return pids


def kill_game():
    for sig, wait in ((signal.SIGTERM, 15), (signal.SIGKILL, 10)):
        for pid in game_pids():
            try:
                os.kill(pid, sig)
            except OSError:
                pass
        end = time.time() + wait
        while game_pids() and time.time() < end:
            time.sleep(1)
        if not game_pids():
            return


def launch(g):
    if g['store'] == 'heroic':
        url = 'heroic://launch?appName=%s&runner=gog' % g['app_name']
        cmd = (['flatpak', 'run', 'com.heroicgameslauncher.hgl'] if g.get('flatpak_heroic') else ['heroic'])
        cmd += ['--no-gui', url]
    elif g['store'] == 'steam':
        cmd = (['flatpak', 'run', 'com.valvesoftware.Steam'] if g.get('flatpak_steam') else ['steam'])
        cmd += ['steam://rungameid/%s' % w3vr.APP_ID]
    else:
        w3vr.die('Unattended start needs a Heroic or Steam install (found: %s).' % g['store'])
    log('start: %s' % ' '.join(cmd))
    out = open(os.devnull, 'wb')
    subprocess.Popen(cmd, stdin=subprocess.DEVNULL, stdout=out, stderr=out, start_new_session=True)


def next_run_dir(output, name):
    os.makedirs(output, exist_ok=True)
    numbers = [int(m.group(1)) for d in os.listdir(output) for m in [re.search(r'_lx(\d+)_', d)] if m]
    nr = max(numbers, default=0) + 1
    path = os.path.join(output, '%s_lx%d_%s' % (datetime.date.today().isoformat(), nr, name))
    os.makedirs(path)
    return path


def snapshot_saves(saves):
    if not os.path.isdir(saves):
        return {}
    return {n: sha256(os.path.join(saves, n)) for n in sorted(os.listdir(saves))
            if os.path.isfile(os.path.join(saves, n))}


def restore_saves(saves, backup, before, run_dir):
    """Put the save folder back to |before|; changed or new files go to run_dir/saves-after-run/."""
    after = snapshot_saves(saves)
    moved = [n for n in after if before.get(n) != after[n]]
    missing = [n for n in before if n not in after]
    if moved:
        keep = os.path.join(run_dir, 'saves-after-run')
        os.makedirs(keep, exist_ok=True)
        for n in moved:
            shutil.move(os.path.join(saves, n), os.path.join(keep, n))
    for n in moved + missing:
        if n in before:
            shutil.copy2(os.path.join(backup, n), os.path.join(saves, n))
    ok = snapshot_saves(saves) == before
    return moved, missing, ok


def settings_diff(before_text, path):
    if before_text is None or not os.path.exists(path):
        return []
    before = w3vr.Ini(before_text).snapshot()
    after = w3vr.Ini.load(path).snapshot()
    changes = []
    for key in sorted(set(before) | set(after)):
        if before.get(key) != after.get(key):
            changes.append('[%s] %s: %s -> %s' % (key[0], key[1], before.get(key), after.get(key)))
    return changes


def apply_ini_sets(ini, assignments):
    for a in assignments:
        m = re.match(r'^([^.=\s]+)\.([^=\s]+)=(.*)$', a)
        if not m:
            w3vr.die('Bad --ini-set value: %s' % a)
        ini.set(m.group(1), m.group(2), m.group(3))


def one_run(g, args, run_dir):
    x64 = g['x64']
    dll, ini_path = os.path.join(x64, 'dxgi.dll'), os.path.join(x64, 'witcher3vr.ini')
    settings = os.path.join(g['docs'], 'dx12user.settings')
    saves = os.path.join(g['docs'], 'gamesaves')
    failures, notes = [], []

    # Pre-checks.
    if game_pids():
        w3vr.die('The Witcher 3 is already running.')
    for required in (dll, ini_path, settings, args.script):
        if not os.path.exists(required):
            w3vr.die('Not found: %s' % required)
    settings_ini = w3vr.Ini.load(settings)
    if (settings_ini.get('Gameplay', 'CrossProgression', '') or '').lower() != 'false':
        w3vr.die('[Gameplay] CrossProgression is not false in %s; refusing to start '
                 '(cross-progression could sync test saves to the account).' % settings)
    with open(settings, 'rb') as f:
        settings_before = f.read().decode('utf-8-sig')

    # Backups.
    backup = os.path.join(run_dir, 'backup')
    os.makedirs(os.path.join(backup, 'gamesaves'))
    shutil.copy2(dll, os.path.join(backup, 'dxgi.dll'))
    shutil.copy2(ini_path, os.path.join(backup, 'witcher3vr.ini'))
    saves_before = snapshot_saves(saves)
    for n in saves_before:
        shutil.copy2(os.path.join(saves, n), os.path.join(backup, 'gamesaves', n))
    original = {p: sha256(p) for p in (dll, ini_path)}
    with open(os.path.join(backup, 'sha256.txt'), 'w') as f:
        for p, h in original.items():
            f.write('%s  %s\n' % (h, os.path.basename(p)))
        for n, h in saves_before.items():
            f.write('%s  gamesaves/%s\n' % (h, n))

    # Leftovers from earlier runs would be mixed into this run's results.
    stale = [n for n in os.listdir(x64) if re.match(r'witcher3vr.*\.log$|witcher3vr_captures$', n)]
    if stale:
        os.makedirs(os.path.join(run_dir, 'stale-before-run'))
        for n in stale:
            shutil.move(os.path.join(x64, n), os.path.join(run_dir, 'stale-before-run', n))

    script_name = 'witcher3vr-test-' + os.path.basename(args.script)
    script_path = os.path.join(x64, script_name)
    with open(args.script, encoding='utf-8') as f:
        expected_captures = 2 * sum(1 for l in f if re.match(r'^\s*\d+\s+capture\b', l))

    started = time.time()
    killed = never_started = False
    try:
        if args.dll:
            shutil.copy2(args.dll, dll)
        shutil.copy2(args.script, script_path)
        ini = w3vr.Ini.load(ini_path)
        ini.set('test', 'record_input', '0')
        ini.set('test', 'input_script', script_name)
        if not args.no_keep_focus:
            ini.set('test', 'keep_focus', '1')
        apply_ini_sets(ini, args.ini_set)
        with open(ini_path, 'wb') as f:
            f.write(ini.text().encode('utf-8'))
        shutil.copy2(ini_path, os.path.join(run_dir, 'witcher3vr.ini.used'))
        dll_hash = sha256(dll)

        launch(g)
        end = time.time() + START_TIMEOUT
        while not game_pids() and time.time() < end:
            time.sleep(2)
        if not game_pids():
            never_started = True
        else:
            log('game running, waiting up to %d s' % args.timeout)
            end = time.time() + args.timeout
            while game_pids():
                if time.time() >= end:
                    killed = True
                    log('timeout, killing the game')
                    kill_game()
                    break
                time.sleep(3)
        log('game gone')
        time.sleep(3)  # let Wine flush and release the log files
    finally:
        # Collect, then restore (also after Ctrl+C or errors).
        for n in os.listdir(x64):
            if re.match(r'witcher3vr.*\.log$|witcher3vr-input-recording.*\.txt$|witcher3vr_captures$', n):
                shutil.move(os.path.join(x64, n), os.path.join(run_dir, n))
        if os.path.exists(script_path):
            shutil.move(script_path, os.path.join(run_dir, script_name))
        if not args.keep_ini:
            shutil.copy2(os.path.join(backup, 'witcher3vr.ini'), ini_path)
        shutil.copy2(os.path.join(backup, 'dxgi.dll'), dll)
        moved, missing, saves_ok = restore_saves(saves, os.path.join(backup, 'gamesaves'), saves_before, run_dir)
    seconds = int(time.time() - started)

    # Restore checks.
    if sha256(dll) != original[dll]:
        failures.append('dxgi.dll NOT restored (sha256 mismatch)')
    if not args.keep_ini and sha256(ini_path) != original[ini_path]:
        failures.append('witcher3vr.ini NOT restored (sha256 mismatch)')
    if not saves_ok:
        failures.append('save folder NOT restored')
    if moved or missing:
        notes.append('saves restored (changed/new moved to saves-after-run/: %s; missing: %s)'
                     % (', '.join(moved) or '-', ', '.join(missing) or '-'))

    # Evaluation, like scripts/smoke-test.ps1.
    log_path = os.path.join(run_dir, 'witcher3vr.log')
    lines = []
    if os.path.exists(log_path):
        with open(log_path, encoding='utf-8', errors='replace') as f:
            lines = f.read().splitlines()
    else:
        failures.append('witcher3vr.log missing (proxy not loaded?)')
    strip = lambda l: re.sub(r'^\[[^\]]*\]\s*', '', l)
    build = next((l for l in lines if 'Game build ' in l), None)
    if build:
        notes.append(strip(build))
    if never_started:
        failures.append('game did not start within %d s' % START_TIMEOUT)
    if killed:
        failures.append('game did not exit within %d s (killed)' % args.timeout)
    if lines and not any('Test mode script loaded' in l for l in lines):
        failures.append('test mode script not loaded')
    written = sum(1 for l in lines if 'Test mode capture written' in l)
    if written != expected_captures:
        failures.append('captures written %d, expected %d' % (written, expected_captures))
    for l in [l for l in lines if re.search(r'\bhook\b.*\bfailed\b|\bfailed\b.*\bhook\b|signature mismatch', l)][:10]:
        if any(e in l for e in EXPECTED_HOOK_FAILURES):
            notes.append('expected under Proton: ' + strip(l)[:120])
        else:
            failures.append('hook failure: ' + strip(l))
    if any('Test mode quit' in l for l in lines):
        notes.append('exit: script quit action')
        if args.expect_self_exit:
            failures.append('game had to be closed by the script quit (menu EXIT path did not work)')
    elif not killed and not never_started:
        notes.append('exit: game closed itself (e.g. menu EXIT), no script quit needed')
    summary_line = [l for l in lines if 'Test mode input at_ms=' in l]
    if summary_line:
        m = re.search(r'xinput_calls=(\d+)', summary_line[-1])
        notes.append('input: xinput9 calls=%s (last summary)' % (m.group(1) if m else '?'))
    swallowed = any('keep_focus swallowed' in l for l in lines)
    if any('Test mode keep_focus installed' in l for l in lines):
        notes.append('keep_focus: installed%s' % (', focus loss hidden from the game' if swallowed else ''))
    if any('Engine dual render INI auto-start queued' in l for l in lines):
        notes.append('stereo: dual render started')
    # Per capture time: were both XR eye images written? Eyes exist only with a stereo pair, i.e. in gameplay;
    # menus and loading screens have none. The game also pauses itself when its window loses focus.
    capture_dir = os.path.join(run_dir, 'witcher3vr_captures')
    times = {}
    for n in (os.listdir(capture_dir) if os.path.isdir(capture_dir) else []):
        m = re.match(r'capture_\d+_(\d+)ms_(f\d|xr_eye[01])\.png$', n)
        if m:
            times.setdefault(int(m.group(1)), set()).add(m.group(2))
    timeline = ['%ds:%s' % (t // 1000, 'eyes' if {'xr_eye0', 'xr_eye1'} <= kinds else 'flat')
                for t, kinds in sorted(times.items())]
    if timeline:
        notes.append('captures: ' + ' '.join(timeline))
    if args.expect_eyes:
        low, high = (int(v) for v in args.expect_eyes.split('-'))
        flat = [t for t, kinds in sorted(times.items())
                if low <= t <= high and not {'xr_eye0', 'xr_eye1'} <= kinds]
        if flat:
            failures.append('no XR eye images at %s s (not in gameplay: menu/pause, focus lost?)'
                            % ', '.join(str(t // 1000) for t in flat))
    for change in settings_diff(settings_before, settings)[:20]:
        notes.append('dx12user.settings changed: ' + change)

    passed = not failures
    summary = ['result: %s' % ('PASSED' if passed else 'FAILED'),
               'run: %s' % os.path.basename(run_dir),
               'store: %s, wine: %s' % (g['store'], g.get('wine', '') or '-'),
               'script: %s' % os.path.basename(args.script),
               'dxgi.dll sha256: %s%s' % (dll_hash, ' (from --dll)' if args.dll else ' (installed)'),
               'duration: %d s' % seconds,
               'captures: %d/%d' % (written, expected_captures)]
    summary += ['ini: %s' % a for a in args.ini_set]
    summary += ['conditions: %s' % args.note] if args.note else []
    summary += ['note: %s' % n for n in notes] + ['failure: %s' % f for f in failures]
    with open(os.path.join(run_dir, 'summary.txt'), 'w', encoding='utf-8') as f:
        f.write('\n'.join(summary) + '\n')
    print('\n'.join(summary))
    print('output: %s' % run_dir)
    return passed


def main():
    p = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    p.add_argument('--script', default=DEFAULT_SCRIPT, help='input script (src/test_input_script.h format)')
    p.add_argument('--dll', help='dxgi.dll to use for the run(s); the installed one is restored afterwards')
    p.add_argument('--name', default='smoke', help='run name suffix (folder <date>_lx<n>_<name>)')
    p.add_argument('--repeat', type=int, default=1, help='number of runs in a row')
    p.add_argument('--timeout', type=int, default=420, help='seconds the game may run before it is killed')
    p.add_argument('--ini-set', action='append', default=[], metavar='section.key=value',
                   help='extra witcher3vr.ini value for the run(s); repeatable')
    p.add_argument('--expect-eyes', metavar='FROM-TO',
                   help='ms range in which every capture must have both XR eye images (gameplay), '
                        'e.g. 90000-216000 for linux-capture-remaster.txt')
    p.add_argument('--expect-self-exit', action='store_true',
                   help='fail when the script quit had to close the game (the script exits through the menu)')
    p.add_argument('--note', help='test conditions for the summary (e.g. "battery, parallel load")')
    p.add_argument('--keep-ini', action='store_true', help='keep the test INI after the run')
    p.add_argument('--no-keep-focus', action='store_true',
                   help='do not set [test] keep_focus=1 (then clicking another window pauses the game)')
    p.add_argument('--output', default=DEFAULT_OUTPUT)
    p.add_argument('--game-dir')
    p.add_argument('--prefix')
    args = p.parse_args()
    args.script = os.path.abspath(args.script)
    if args.script == os.path.abspath(DEFAULT_SCRIPT):
        args.expect_eyes = args.expect_eyes or DEFAULT_EXPECT_EYES
        args.expect_self_exit = True
    if args.dll:
        args.dll = os.path.abspath(args.dll)
        if not os.path.exists(args.dll):
            w3vr.die('Not found: %s' % args.dll)
    if args.game_dir:
        w3vr.GAME_OVERRIDE['game'] = args.game_dir
    if args.prefix:
        w3vr.GAME_OVERRIDE['prefix'] = args.prefix
    g = w3vr.require_game()
    if w3vr.game_build(g) != '501':
        w3vr.die('Only Remastered 5.01 is set up for unattended Linux runs.')

    results = []
    for i in range(args.repeat):
        name = args.name if args.repeat == 1 else '%s-%d' % (args.name, i + 1)
        run_dir = next_run_dir(args.output, name)
        log('run %d/%d -> %s' % (i + 1, args.repeat, run_dir))
        results.append(one_run(g, args, run_dir))
    if args.repeat > 1:
        print('runs passed: %d/%d' % (sum(results), len(results)))
    sys.exit(0 if all(results) else 1)


if __name__ == '__main__':
    main()
