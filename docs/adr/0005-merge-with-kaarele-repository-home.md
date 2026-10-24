# 0005: Possible merge with kaarele's project; home of a combined repository

- Status: Proposed (both maintainers would like to combine the projects; nothing is agreed yet)
- Date: 2026-10-23
- Decided by: the maintainer and kaarele (open)

## Context

This fork (`joschi127/witcher3-vr`) and
[kaarele/witcher3-remastered-vr](https://github.com/kaarele/witcher3-remastered-vr) work on the
same mod for Remastered 5.x. Both maintainers have said they would like to combine the two
projects into one; whether and how that happens is not settled. This fork already takes over
kaarele's changes feature by feature ([ADR 0004](0004-adopt-kaarele-changes-feature-by-feature.md)),
which is useful on its own and does not depend on a merge.

Links to this repository are already public and in use, for example in the VR modding community
on Discord: `https://github.com/joschi127/witcher3-vr` and the release
`https://github.com/joschi127/witcher3-vr/releases/tag/v0.9.8v2-remastered.1`. They should keep
working if possible.

Facts checked on 2026-10-23 (GitHub documentation and API):

- **Transferring a repository** moves issues, pull requests, releases, wiki, stars, watchers and
  history. Web and git URLs of the old location redirect to the new one (clone, fetch, push
  included); GitHub Pages do not. The redirect is lost for good if a repository or fork is later
  created at the old location (`joschi127/witcher3-vr`). The documentation does not explicitly
  mention release asset download URLs (`/releases/download/...`).
- The target account must not already have a repository with the same name or **a fork in the
  same fork network**. `kaarele/witcher3-remastered-vr` is a GitHub fork of
  `joschi127/witcher3-vr` (network root `tig3rmast3r/witcher3-vr`), so a transfer to kaarele's
  personal account is blocked while that fork exists there.
- A transfer to a personal account must be accepted by the recipient within one day. The previous
  owner stays on as a collaborator.
- A personal-account repository has only an owner and collaborators. Collaborators can push,
  merge pull requests, manage issues and create releases, but cannot change settings, manage
  collaborators, archive, transfer or delete the repository. Admin or maintain roles for a second
  person exist only in an organization.

## Decision

Intended by both maintainers, not agreed yet:
- one combined repository for both projects;
- both maintainers with maintainer permissions in it.

Proposed by the maintainer, not agreed yet: do not create the combined repository as a new one,
but transfer the existing `joschi127/witcher3-vr`, so that its frequently used public links keep
redirecting instead of running into nothing.

Open: where the combined repository lives. Options, assuming the transfer proposal:

1. **A new GitHub organization, owned by both** (preferred by the maintainer).
   `joschi127/witcher3-vr` is transferred there; both are owners, so both have full maintainer
   permissions. kaarele's fork can stay on his account until it is archived with a pointer, so no
   existing link breaks. The project name is not tied to one person.
2. **kaarele's personal account** (preferred by kaarele; fine for the maintainer as well).
   kaarele first removes his fork from his account (deletes it, or transfers it to another
   account), because the fork blocks the transfer; then `joschi127/witcher3-vr` is transferred to
   him and can be renamed. The links of his current repository stop working if it is deleted (he
   can keep a "moved" note elsewhere, as he did for `witcher3-remastered-vr-old`). joschi127
   becomes a collaborator with write access, not an admin.

Open: the organization's name (for option 1). It should not be specific to The Witcher 3: both
maintainers may work on more VR gaming projects, with a focus on VR gaming on Linux (especially
for kaarele), but not only on Linux. The maintainer's two favourites, both free on GitHub on
2026-10-23 and not used by a known VR project:
- `second-eye`: the mods render the second eye that flat games lack
  (`github.com/second-eye/witcher3-vr`);
- `stereo-works`: a workshop for stereo VR versions of flat games
  (`github.com/stereo-works/witcher3-vr`).

Names close to existing VR projects or trademarks (Flat2VR, UEVR, LVRA, Monado, WiVRn, ALVR,
OpenXR, OpenVR) are avoided. A name is only reserved once the organization is created.

## Consequences

If the transfer proposal is taken:
- Never create a new repository or fork named `joschi127/witcher3-vr` after the transfer: that
  deletes the redirects.
- Before the real transfer, test with a throwaway repository and release that release download
  URLs redirect as well.
- After the transfer, local clones update their remote (`git remote set-url origin ...`); docs,
  release notes and the launcher's links are updated to the new location.
- With option 1, both can change settings and collaborators; with option 2, only kaarele can.

If a new repository is created instead, `joschi127/witcher3-vr` stays where it is with a clear
pointer to the new home (README, release notes, a final release), because its links cannot
redirect.
