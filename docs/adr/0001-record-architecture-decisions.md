# 0001: Record architecture decisions

- Status: Accepted
- Date: 2026-10-23
- Decided by: the maintainer

## Context

The fork has grown from a port of one mod to a second game version into a project with its own
releases, a parallel project to merge with (kaarele/witcher3-remastered-vr) and several people
and AI agents working on it. Decisions and their reasons were spread over the guidelines, the
port status log, issues and git history, so the reasons behind a rule were hard to find.

## Decision

Record decisions that shape the code or the project as ADRs in `docs/adr/`, one Markdown file
each, using [template.md](template.md). The guidelines keep the rules short and link to the ADR
that explains them.

## Consequences

- Reasons can be looked up instead of reconstructed; new contributors and agents read them once.
- Writing an ADR takes a few minutes per decision; small fixes do not need one.
- Accepted ADRs stay as written; changes of mind are new ADRs, so the history stays readable.
