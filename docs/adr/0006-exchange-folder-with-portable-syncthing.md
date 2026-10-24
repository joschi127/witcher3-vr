# 0006: Exchange folder with portable Syncthing

- Status: Proposed (not set up yet; so far git and messages between the sessions were enough)
- Date: 2026-10-23
- Decided by: the maintainer (open)

## Context

The maintainer tests on a Windows PC and, for Linux/Proton, on a Debian laptop, each with its own
Claude Code session. Both should be able to test the same state: the same test saves, input
scripts, DLL builds, and compare logs and captures. Saves, settings and logs must not go into git
(data hygiene), GOG cloud saves are off on purpose (they could carry test saves into a real
playthrough), and the maintainer prefers a tool that needs no installation.

Options considered:
- Syncthing, portable: one executable per platform, keeps a folder in sync directly between the
  machines (LAN, encrypted; global discovery and relays optional).
- croc: one executable, sends single files with a code phrase; both sides must be active at the
  same time; uses a public relay by default (end-to-end encrypted).
- LocalSend: LAN file sending with a GUI; fine by hand, awkward for two Claude sessions.
- A Windows share (SMB) mounted on Linux, or `scp`/`rsync` over SSH: work, but need setup on the
  laptop (SSH server) or depend on the PC being on, and every transfer is manual.
- A separate private git repository for test data: possible, but heavier, and the data sits at a
  hosting provider.

## Decision

Use a git-ignored folder `local/exchange/` in each clone, kept in sync with portable Syncthing
(unpacked under `local/tools/syncthing/`, configuration next to it, no installation), set up when
needed by the AI agent of each session with the maintainer's go-ahead. Only that folder is shared. Saves are copied into a game only with the backup routine of the guidelines;
settings files are exchanged only deliberately, one at a time.

## Consequences

- A save, script or DLL placed on one machine is on the other shortly after, without the
  maintainer moving files by hand.
- Syncthing opens a network port; the firewall asks once on first start.
- Deletions sync too; Syncthing's file versioning (`.stversions`) is a safety net for mistakes.
- The exchange is optional: single files can still be sent any other way, for example with croc.
