# Chaos Edition

Chaos Edition is an unofficial fork based on Return to the Roots. It exists to experiment with ideas that may not belong in the upstream project, while keeping the codebase close enough to upstream/master that regular rebases and selective upstream contributions remain practical.

This document is a coordination point for the fork. It does not change gameplay, graphics, AI, build behavior, or asset handling.

## Technical Base

- `upstream/master` remains the technical base for this fork.
- The `chaos-edition` branch should stay rebaseable or mergeable with upstream with minimal conflict churn.
- Foundation work should prefer documentation, metadata, settings, isolated addons, and feature gates over direct edits to core behavior.
- Neutral bug fixes, build fixes, portability fixes, and maintenance cleanups should be kept small enough to upstream as separate pull requests.
- Chaos-only gameplay, content, balancing, graphics, and presentation changes stay in this fork unless upstream explicitly wants them.

## Change Boundaries

Chaos Edition should avoid mixing unrelated goals in the same change. A useful rule:

- If a change would make upstream better without changing the game's identity, keep it neutral and upstreamable.
- If a change changes gameplay, content, graphics, tone, balance, or player-facing identity, keep it clearly marked as Chaos-only.
- If a change can be isolated behind a setting, addon, scenario rule, build option, or feature flag without distorting the upstream code, isolate it.
- If a change needs original game assets, do not bundle or replace them blindly. Respect the existing expectation that users provide required original assets.

## Documents

- [Roadmap](docs/chaos-edition/ROADMAP.md)
- [Upstream Policy](docs/chaos-edition/UPSTREAM_POLICY.md)
- [Feature Requirements](docs/chaos-edition/FEATURES.md)
