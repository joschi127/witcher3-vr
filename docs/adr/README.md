# Architecture decision records

Decisions about this fork that shape the code or the project and are hard to reverse. When to
write one: see [Architecture decision records](../guidelines.md#architecture-decision-records) in
the guidelines.

## How to add one

1. Copy [template.md](template.md) to `NNNN-short-title.md` with the next free number.
2. Fill in context, decision and consequences. Keep it short; link to code, docs, issues or
   commits instead of repeating them.
3. Status `Proposed` while it is open, `Accepted` once the maintainer agrees. An accepted ADR is
   not rewritten: a later change is a new ADR that supersedes it, and the old one gets status
   `Superseded by NNNN`.
4. Add it to the list below in the same commit.

Committed ADRs follow the data hygiene rules of the guidelines: no local paths, logs or
reverse-engineering material.

## Records

| No. | Title | Status |
|---|---|---|
| [0001](0001-record-architecture-decisions.md) | Record architecture decisions | Accepted |
| [0002](0002-one-codebase-for-404-and-5x.md) | One codebase for Next-Gen 4.04 and Remastered 5.x | Proposed |
| [0003](0003-build-guard-and-per-build-address-tables.md) | Build guard and per-build address tables | Accepted |
| [0004](0004-adopt-kaarele-changes-feature-by-feature.md) | Adopt kaarele's changes feature by feature | Accepted |
