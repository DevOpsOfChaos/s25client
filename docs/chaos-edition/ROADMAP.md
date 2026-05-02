# Chaos Edition Roadmap

Chaos Edition is an unofficial edition of Return To The Roots. The roadmap is intentionally staged so the fork can gain identity without losing upstream compatibility.

## Stage 0: Foundation

Goal: define the fork layer without changing behavior.

- Add fork documentation and policies.
- Keep `upstream/master` as the technical base.
- Avoid gameplay, graphics, AI, asset, or balance changes.
- Identify existing extension points for settings, addons, feature flags, scenario rules, and metadata.
- Keep changes reviewable as small patches.

Exit criteria:

- Fork purpose and boundaries are documented.
- Upstream contribution rules are documented.
- No gameplay, graphics, AI, or asset behavior has changed.

## Stage 1: Isolation Points

Goal: prepare safe places for Chaos-only behavior.

- Prefer existing settings and addon systems before adding new mechanisms.
- Add feature flags only where they reduce long-term conflict risk.
- Keep default behavior aligned with upstream unless a Chaos setting is explicitly enabled.
- Document each Chaos-only switch near the implementation and in user-facing fork docs.
- Avoid large cross-cutting rewrites.

Exit criteria:

- Chaos-only experiments can be enabled explicitly.
- Upstream-compatible defaults remain intact.
- Neutral infrastructure remains separable from fork-specific behavior.

## Stage 2: Fork Identity

Goal: add Chaos-only features while keeping the upstream boundary visible.

- Prototype gameplay/content changes behind settings, addons, scenario rules, or feature flags when practical.
- Keep balance experiments scoped and reversible.
- Avoid replacing upstream assets or bundling original game assets.
- Separate neutral fixes from fork-only commits.
- Rebase or merge from upstream regularly enough that conflicts stay understandable.

Exit criteria:

- Chaos Edition has distinct opt-in behavior.
- Upstream compatibility remains actively maintained.
- Fork-only behavior is easy to locate and audit.

## Stage 3: Release Discipline

Goal: make the fork usable without creating legal, maintenance, or identity confusion.

- Clearly label builds and documentation as unofficial.
- Keep original project attribution visible.
- Document required original assets instead of bundling or replacing them blindly.
- Track known differences from upstream.
- Maintain a checklist for rebasing, smoke testing, and release notes.

Exit criteria:

- Users can tell what is upstream and what is Chaos-only.
- Maintainers can rebase with predictable conflict areas.
- Release artifacts do not imply upstream endorsement.

