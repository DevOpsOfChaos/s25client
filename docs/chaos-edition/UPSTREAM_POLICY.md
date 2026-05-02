# Chaos Edition Upstream Policy

Chaos Edition is an unofficial fork/edition of Return To The Roots. The fork should stay technically close to `upstream/master` while reserving space for changes that are not appropriate for upstream.

## Base Branch

- `upstream/master` remains the technical base.
- The fork should regularly import upstream changes.
- Fork changes should be structured to minimize recurring merge and rebase conflicts.
- Existing upstream project identity, licensing, and attribution must remain intact.

## Upstreamable Changes

Neutral changes may be upstreamed as small, focused pull requests. Good candidates include:

- Crash fixes.
- Deterministic bug fixes.
- Build and CI fixes.
- Platform compatibility fixes.
- Test improvements.
- Documentation corrections.
- Refactors that reduce risk without changing player-facing behavior.

Upstreamable work should not depend on Chaos-only behavior. Keep it separate from fork-only commits so it can be reviewed on its own.

## Fork-Only Changes

The following should stay in Chaos Edition unless upstream explicitly asks for them:

- Gameplay changes.
- Balance changes.
- AI behavior changes.
- New or altered content.
- Graphics or presentation changes.
- Tone, naming, or identity changes tied to Chaos Edition.
- Experimental mechanics that alter default upstream behavior.

Fork-only changes should be easy to find, disable, or remove. When practical, isolate them behind settings, addons, scenario rules, build options, or feature flags.

## Defaults and Feature Gates

- Default behavior should remain upstream-compatible unless the fork intentionally chooses a new default.
- Chaos-only features should be opt-in when practical.
- Feature flags should not become a dumping ground for unclear design. If a feature cannot be explained, scoped, and tested, the flag is hiding a planning problem.
- Settings and addons should use existing project patterns before new infrastructure is added.

## Assets

Chaos Edition must not bundle, replace, or redistribute original game assets blindly.

- Respect the existing model that users provide required original assets.
- Do not overwrite user-provided assets without explicit intent and documentation.
- New fork-owned assets should be clearly separated from original game assets.
- Any asset replacement plan needs legal, technical, and user-facing review before implementation.

## Commit Hygiene

- Keep neutral upstreamable changes separate from Chaos-only changes.
- Prefer small commits with one reason to exist.
- Avoid broad formatting churn in files that will frequently receive upstream changes.
- Document intentional divergence when it is introduced.
- Do not use existing upstream PR branches as a workspace for Chaos Edition work.

