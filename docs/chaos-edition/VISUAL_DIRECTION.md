# Visual Modernization Direction

Chaos Edition should pursue a modernized pixel-art presentation, not generic "HD graphics".

The long-term goal is better scaling, clearer readability, and clean behavior on modern monitor sizes while keeping the visual identity of an old strategy game. The original strategy-game feel should remain recognizable. Pixel-art identity is a core constraint, not a temporary limitation to be erased by higher resolution work.

## Goals

- Preserve the classic strategy-game feel.
- Preserve the pixel-art identity of terrain, buildings, workers, animals, resources, icons, and UI framing.
- Improve scaling quality so the game remains readable across modern desktop and laptop displays.
- Support modern monitor sizes cleanly without making the presentation feel like a different game.
- Make UI surfaces more understandable and user-friendly without changing game rules.
- Keep future custom Chaos content visually compatible with the established pixel-art direction.

## Non-goals

- No realistic redesign.
- No vector-style or mobile-game look.
- No gameplay effects.
- No savegame, network, or replay effects.
- No mandatory HD asset packs.
- No asset replacement before the rendering and scaling foundation is stable.
- No CMake, packaging, binary identity, or executable branding changes as part of visual direction work.

## Visual Principles

- Preserve silhouettes. Buildings, workers, wares, flags, animals, and terrain features should still be identifiable by their original shapes.
- Preserve tile readability. Improvements must make map state easier to read, not hide the tile grid, resource cues, ownership cues, roads, building states, or unit movement.
- Avoid over-smoothing pixel art. Smooth filtering can help some displays and preferences, but it must not become the only or default identity of the fork.
- Prefer crisp scaling where appropriate. Pixel/sharp presentation is the baseline for assets whose readability depends on hard pixel edges.
- Allow smooth filtering as an optional local display preference. Smooth filtering is a user display preference, not a content or compatibility mode.
- Improve UI clarity without changing game rules. Clearer icons, text sizing, spacing, and HiDPI behavior are presentation work only.

## Technical Phases

### Phase 1: Local rendering and scaling options

Keep first improvements local to display settings. The existing texture filtering option belongs here:

- Pixel / sharp
- Smooth

These options are local presentation preferences stored under `Settings::video`.

### Phase 2: UI and HiDPI readability

Improve menu, window, and in-game UI readability for modern display sizes and DPI behavior. This includes clearer sizing, spacing, and presentation rules, but not gameplay or simulation changes.

### Phase 3: Pixel-perfect and integer scaling investigation

Investigate whether pixel-perfect or integer scaling can be implemented without breaking mouse mapping, zoom behavior, viewport logic, or normal window resizing. This should be treated as renderer and presentation work, not as an asset-pack task.

The investigation starts with pure viewport math and documentation, not with a user-facing option. A future local option should only be exposed after the project has proven coordinate correctness for a centered integer-scaled presentation rectangle. The likely mode shape is:

- `Auto`
- `Integer / pixel-perfect`
- `Free / current behavior`

Texture filtering remains a separate local display preference. `Pixel / sharp` complements integer scaling, while `Smooth` intentionally softens texture sampling and must not be treated as a substitute for pixel-perfect presentation.

Visible scaling or presentation prototypes must follow the [Chaos Visual Review Workflow](VISUAL_REVIEW.md) before they are committed as active runtime behavior.

### Phase 4: Optional asset override layer with fallback

Only consider a Chaos-specific asset override layer after the rendering and scaling foundation is stable and after fallback, precedence, packaging, and compatibility rules are documented. Any override path must fall back cleanly to existing assets.

### Phase 5: Custom Chaos assets and content

Custom Chaos assets and content should wait until the visual style, scaling behavior, and override rules are stable enough to avoid rework. New content should follow the modernized pixel-art presentation direction instead of drifting toward realism, vector art, or a mobile remake style.

## Hard Boundary

Display options are local-only. They must not use or modify:

- Addons
- `GlobalGameSettings`
- `.chaos` required features
- Savegames
- Network synchronization
- Replay data
- Simulation state

Visual modernization must remain presentation-side unless a future document explicitly defines a separate, reviewed boundary. Graphics settings must not affect map interpretation, command generation, deterministic game state, multiplayer compatibility, replay playback, or savegame compatibility.

## Content Timing

New custom content should not start until the visual, style, and pipeline foundation is stable enough. Starting asset production too early would lock Chaos Edition into accidental style decisions and create avoidable rework. The correct order is rendering/scaling foundation first, style and override rules second, custom assets third.
