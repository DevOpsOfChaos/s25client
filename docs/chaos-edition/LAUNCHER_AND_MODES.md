# Chaos Edition Launcher And Modes

Chaos Edition is an unofficial fork/product line of Return To The Roots. This document defines the planned launcher, appearance, rules-mode, settings, and compatibility model before implementation begins.

## Purpose

The Chaos Edition launcher should provide a distinct Chaos-branded entry point while preserving a clear path to RTTR-compatible behavior where practical.

The launcher may own fork-specific presentation, including texts, graphics, loading screens, menus, visual identity, and mode selection. It must also make the difference between visual appearance and gameplay rules explicit, because confusing those concepts would create broken saves, misleading map compatibility, and user-facing behavior that is impossible to reason about.

The target model is:

- Chaos Edition has its own launcher and branding layer based on the RTTR launcher.
- Users can choose a Chaos appearance or a classic RTTR-like appearance inside the Chaos launcher.
- Users can choose a high-level rules profile separately from appearance.
- Maps and saves declare the Chaos features they require.
- The launcher or loader blocks unsupported maps and saves instead of silently changing their meaning.

## Non-goals

This document does not implement the launcher, change game logic, replace assets, or define final UI artwork.

The launcher and mode model must not be used as an excuse to:

- Replace original RTTR/S2 assets blindly.
- Bundle assets without clear rights.
- Pretend Chaos Edition is an official RTTR release.
- Allow every possible checkbox combination just because the UI can expose it.
- Treat a classic-looking UI as proof that the game is running RTTR-compatible logic.
- Silently reinterpret Chaos maps or saves as RTTR-compatible content.

## Appearance vs Rules Mode

Appearance and gameplay rules are separate concepts.

Appearance controls presentation:

- Launcher branding.
- Menu texts and graphics.
- Loading screens.
- UI skinning and visual identity.
- Classic RTTR-like presentation inside the Chaos launcher.

Rules mode controls behavior:

- Gameplay rules.
- Addon availability.
- Scenario and map feature support.
- Save compatibility.
- Simulation assumptions.
- Any Chaos-only mechanics that affect outcomes.

A classic RTTR-like appearance must not automatically mean RTTR-compatible game logic. A user may run Chaos rules with a classic look, and may run RTTR-compatible rules from the Chaos launcher. The selected rules profile, not the selected appearance, decides compatibility.

Planned high-level rules profiles:

- `RTTR-compatible`: Use behavior intended to remain compatible with upstream RTTR expectations where practical.
- `Chaos`: Enable Chaos Edition behavior through explicit feature gates, settings, addons, or dedicated data paths.

Implementation note: the codebase now contains Chaos Mode Configuration v1. The selected rules profile is persisted in
the user settings as a narrow configuration value, and the Chaos options tab exposes a selector for these two profile
names. It is still metadata only at this stage and is not wired into gameplay, map loading, saves, networking, or
launcher behavior.

The UI must label these choices clearly. If a user picks a classic appearance while keeping Chaos rules enabled, the launcher should not imply that maps, saves, networking, or replays are RTTR-compatible.

## Launcher and Branding Layer

The Chaos launcher is a fork-specific shell around the RTTR launcher foundation. It may change presentation while keeping upstream-compatible behavior available where practical.

Allowed launcher responsibilities:

- Present Chaos Edition as an unofficial fork.
- Select appearance/theme.
- Select a high-level rules profile.
- Surface compatibility warnings before launch.
- Route users to Chaos-specific settings without crowding normal RTTR settings.
- Check map and save feature requirements before opening content.
- Keep attribution and licensing information visible.

The launcher should not hide fork identity. A Chaos build with a classic RTTR-like appearance still needs to be identifiable as Chaos Edition, because pretending otherwise creates support, compatibility, and licensing confusion.

## Chaos Settings Tab

Chaos-specific features should live under a dedicated Chaos settings tab instead of competing with normal RTTR settings.

The normal settings area should remain focused on RTTR-style configuration. The Chaos tab should contain fork-only experiments, feature flags, addons, compatibility toggles, and settings that change Chaos behavior.

The Chaos settings tab should:

- Group Chaos-only features under an explicit fork label.
- Show whether a feature affects compatibility.
- Explain when a setting requires Chaos rules mode.
- Disable or hide unsupported combinations.
- Prefer curated presets over a flat list of dangerous checkboxes.
- Preserve RTTR-compatible defaults when the selected rules profile requires them.

The tab must not become a dumping ground. If a setting changes simulation behavior, map requirements, save compatibility, or network compatibility, it needs an explicit compatibility story before it is exposed.

## Feature Requirements

Chaos features should be isolated through one or more explicit mechanisms:

- Feature flags.
- Addons.
- Settings.
- Scenario rules.
- Separate data paths.
- Map metadata.
- Save metadata.

Every Chaos-only feature that affects loading or simulation should have a stable requirement identifier. Maps and saves can then declare the features they need, and the launcher or loader can determine whether the selected rules profile supports them.

Implementation note: the codebase now contains a minimal non-persistent feature requirement metadata skeleton with stable
Chaos feature keys and deterministic missing-feature calculation. It is not wired into map loading, save loading,
networking, persistence, or compatibility blocking yet.

Implementation note: the codebase also contains a minimal compatibility decision skeleton that evaluates a selected
rules profile, supported feature set, and required feature list for future map/save gates. It is not wired into real
loaders yet.

Feature requirements should record at least:

- A feature identifier.
- The minimum version or capability level if versioning is needed.
- Whether the requirement is mandatory or optional.
- Whether the feature affects deterministic simulation.
- The rules profiles that can provide the feature.

Unsupported feature combinations should be blocked early. The wrong answer is a permissive UI that allows incoherent states and leaves the engine to fail later.

## Map and Save Compatibility

Maps and saves may require specific Chaos features. The launcher or loader must block opening them when the selected mode does not provide those features.

Required behavior:

- A Chaos map with mandatory Chaos features must not be loaded as an RTTR-compatible map.
- A Chaos save with mandatory Chaos features must not be resumed under RTTR-compatible rules.
- Missing mandatory features must produce a clear error before content is opened.
- Optional features may be ignored only if the map or save explicitly marks them optional and the fallback behavior is documented.
- The loader must not silently downgrade, reinterpret, or partially load Chaos content as RTTR-compatible content.

The compatibility decision should be based on declared requirements and selected rules mode, not on filename conventions, visual theme, or user assumptions.

When practical, keep both RTTR-compatible and Chaos logic available. The goal is not to erase upstream behavior; the goal is to make the fork boundary explicit and enforceable.

## Asset and Branding Rules

Original RTTR/S2 assets must not be blindly replaced, repackaged, or bundled as Chaos assets.

New Chaos assets must be:

- Owned by the project or contributor.
- Generated with clear rights.
- Licensed for redistribution.
- Attributed where the license requires attribution.
- Kept separate from original asset paths when separation reduces legal or maintenance risk.

The Chaos launcher may provide new graphics, loading screens, icons, menu art, and text, but asset provenance must be reviewable. If asset rights are unclear, the asset does not ship.

Branding rules:

- Chaos Edition must be labeled as an unofficial fork.
- RTTR attribution must remain visible where appropriate.
- The launcher must not imply upstream endorsement.
- Classic RTTR-like appearance is a compatibility and familiarity option, not a claim of official identity.

## Implementation Phases

### Phase 1: Documentation And Decisions

- Define launcher responsibilities.
- Define appearance versus rules mode.
- Define high-level rules profiles.
- Define compatibility and asset rules.
- Avoid code, build, gameplay, or asset changes.

### Phase 2: Metadata And Validation Design

- Define feature requirement identifiers.
- Define where map and save requirements are stored.
- Define how rules profiles advertise supported features.
- Define loader errors for missing or incompatible requirements.
- Decide which combinations are unsupported instead of exposing them as free-form settings.

### Phase 3: Launcher Shell

- Introduce Chaos launcher identity.
- Add appearance selection.
- Add rules profile selection.
- Add the dedicated Chaos settings tab.
- Add visible unofficial-fork labeling and attribution.

### Phase 4: Compatibility Enforcement

- Enforce feature requirements for maps and saves.
- Block unsupported map and save loads before simulation begins.
- Preserve RTTR-compatible behavior where practical.
- Add user-facing diagnostics for missing Chaos features.

### Phase 5: Asset And Release Discipline

- Add only properly owned, generated, or licensed assets.
- Keep asset provenance documented.
- Verify release artifacts do not imply official RTTR status.
- Track known differences between RTTR-compatible and Chaos behavior.

## Maintainer Rules

Maintainers should treat this model as a boundary, not a suggestion.

- Do not couple appearance to gameplay rules.
- Do not make classic appearance imply RTTR-compatible logic.
- Do not load Chaos maps or saves under RTTR-compatible rules unless all mandatory requirements are provided.
- Do not silently downgrade, reinterpret, or partially load incompatible content.
- Do not expose unsupported feature combinations just because the implementation can technically toggle them.
- Keep Chaos-specific features behind explicit flags, addons, settings, scenario rules, or separate data paths.
- Keep RTTR-compatible and Chaos logic available where practical.
- Keep Chaos settings in a dedicated Chaos tab.
- Keep original project attribution visible.
- Do not bundle or replace original RTTR/S2 assets without clear rights.
- Do not describe Chaos Edition as official RTTR.

The strategic failure mode is pretending that branding, theme, rules, and compatibility are one thing. They are not. Keep them separate in the UI, metadata, loader, and release process.
