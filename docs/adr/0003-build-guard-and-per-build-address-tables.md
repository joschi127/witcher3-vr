# 0003: Build guard and per-build address tables

- Status: Accepted
- Date: 2026-10-10 (build guard), 2026-10-12 (address table)
- Decided by: the maintainer

## Context

Upstream patches game functions at raw addresses for one game build and has no version check.
On another build (Remastered 5.x) those hooks land on unrelated code; six of them are installed
unconditionally at startup. The fork has to support 4.04 and 5.x side by side without risking
either.

## Decision

- `src/game_build.h` identifies the running `witcher3.exe` (PE timestamp and image size). Engine
  hooks run only on recognized builds; unknown builds get none.
- Build-dependent addresses, signatures and layout values live in one table per build in
  `src/engine_addresses.h`. Every 5.x entry carries a byte signature that is checked before the
  hook is installed. 4.04 values stay unchanged.
- 5.x hooks are enabled one by one, each with log and smoke-test verification.

## Consequences

- Unmodified upstream builds must never be used on 5.x; only builds with the guard may.
- A new game patch disables the engine hooks until its table is added, instead of crashing.
- Every new engine hook needs an entry per supported build (or an explicit "not on this build").
