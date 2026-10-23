# 0002: One codebase for Next-Gen 4.04 and Remastered 5.x

- Status: Proposed
- Date: 2026-10-23 (open since the start of the port)
- Decided by: open; to be settled with the upstream author and kaarele

## Context

The fork started as a port of upstream (Next-Gen 4.04 only) to Remastered 5.x. Since 2026-10-13
both versions run from one codebase: a build guard selects per-build addresses and behaviour
([ADR 0003](0003-build-guard-and-per-build-address-tables.md)). The upstream author said in
upstream issue #27 that he is unsure about supporting both game versions in one mod and sees the
Remaster as a chance to clean up the code. kaarele's parallel project supports 5.01 only and
dropped the 4.04 DLC bundle.

Options:
- One codebase for both versions (current state): 4.04 users keep getting fixes, shared code is
  fixed once; every change has to be checked on both versions.
- A Remaster-only codebase: simpler code and tests, 4.04-only paths can be removed; 4.04 stays
  with upstream.

## Decision

Open. Until it is decided, keep 4.04 working: per-build tables, 4.04 values unchanged, 4.04
regression checks where a change can affect it.

## Consequences

- Changes taken over from 5.01-only work (such as kaarele's) need a 4.04 check or a 5.01-only
  guard.
- The decision affects how a merged project with kaarele is set up.
