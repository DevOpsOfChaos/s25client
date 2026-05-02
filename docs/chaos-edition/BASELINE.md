# Chaos Edition Technical Baseline

Chaos Edition is an unofficial fork/product line of Return To The Roots. This document records the current technical baseline before fork-specific behavior is added.

## Technical Base

- Chaos Edition is currently based on `upstream/master`.
- The local Chaos worktree is:

```text
C:\Users\mries\Documents\LocalRepos\s25client-chaos-edition
```

- The upstream PR worktree remains separate:

```text
C:\Users\mries\Documents\LocalRepos\s25client
```

- Do not use the upstream PR worktree as the working tree for Chaos Edition changes.
- Keep neutral upstreamable work separate from Chaos-only commits.

## Product Identity

- Chaos Edition is a fork/product line, not an upstream replacement.
- Existing upstream project identity, licensing, and attribution remain intact.
- Fork identity should be introduced deliberately and documented where it changes user-facing behavior.
- The baseline phase does not redefine gameplay, balance, AI, graphics, content, or assets.

## Local Development Environment

- Windows / PowerShell is the primary development environment.
- Visual Studio 2022 Debug is the known local build style.
- The local build folder convention is:

```text
.\build-vs-x64-debug-local
```

- `clang-format` 10.0.0 is required for changed C++ files.
- Formatting requirements apply only when C++ files are changed. This baseline document is documentation-only.

## Local Validation Expectations

Before pushing Chaos Edition work, run:

```powershell
git branch --show-current
git status --short
git diff --name-only upstream/master...HEAD
git diff --stat upstream/master...HEAD
```

For documentation-only changes, run the relevant Git checks for whitespace and staged content before committing. Do not claim tests, builds, formatters, or other validation unless they actually ran.

## Known Non-Goals For The Baseline Phase

- No gameplay changes.
- No balance changes.
- No AI behavior changes.
- No graphics or presentation changes.
- No asset bundling, replacement, or redistribution changes.
- No build-system changes.
- No new feature flags or settings.
- No refactors.
- No changes in the separate upstream PR worktree.

The baseline phase exists to make the fork boundary explicit. Treating documentation as a substitute for design would be fake progress; implementation decisions still need their own scope, review, and validation when Chaos-only behavior begins.
