# Copilot instructions

Read [docs/guidelines.md](../docs/guidelines.md) before working in this repository. It covers the
build, what the tests do and do not cover, porting rules, data hygiene and the git workflow.
Current state and to-dos are in the git-ignored `local/NOTES.md`; read it first if present.

Rules that always apply:
- Ask the maintainer before side effects: installs, pushes, deletes, anything in game
  directories or `Documents\The Witcher 3`.
- Never commit game files, settings files, logs, reverse-engineering artifacts, local paths or
  exact hardware identification (see "Data hygiene").
- No AI attribution in commits or PR descriptions.
