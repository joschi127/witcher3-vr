# 0004: Adopt kaarele's changes feature by feature

- Status: Accepted
- Date: 2026-10-22
- Decided by: the maintainer, in contact with kaarele

## Context

[kaarele/witcher3-remastered-vr](https://github.com/kaarele/witcher3-remastered-vr) (MIT) builds
on this fork's `remaster` branch and adds Linux/Proton support and many 5.01 improvements
(performance, per-eye upscaling, HUD and camera fixes). Much of it arrived in one large commit,
later work in smaller ones; his project supports 5.01 only and has not been tested on 4.04. Both
sides want one combined version. Splitting his work into pull requests would be extra work for
him.

## Decision

This fork takes over kaarele's changes itself, feature by feature, from his latest state (not
from earlier intermediate versions):
- one pull request per feature against `remaster`, crediting his repository in the commit messages;
  the description has a summary, the test results (including measured gains) and a list of
  everything not taken over 1:1; pull requests are stacked only where a feature depends on an
  earlier one, otherwise they target `remaster` directly;
- his code is taken over unchanged where possible; deviations stay as small as possible (needed
  for this fork's layout, 4.04, the features taken over so far, or to fix a problem) and are
  listed in the pull request;
- everything that is an improvement without drawbacks for all users is taken over, or fixed until
  it is;
- each step builds and passes CTest, is checked with smoke/null-HMD runs on 5.01 and, where it
  can affect it, 4.04, and goes to the maintainer's headset test (batched where that makes sense,
  see the guidelines);
- 4.04 behaviour stays unchanged unless a change is verified there
  ([ADR 0002](0002-one-codebase-for-404-and-5x.md)).

## Consequences

- His code is kept close to the original (same names, tags and switches), so later changes of his
  can be taken over the same way.
- His measurement and diagnostic infrastructure comes along where features depend on it, off by
  default.
- Fixes found on this side are offered back to him.
