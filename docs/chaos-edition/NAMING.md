# Chaos Edition Naming Policy

This document defines naming and branding rules for Chaos Edition. The goal is to keep the public product identity clear without implying official Return to the Roots status or spreading branding into places where stable technical identifiers are better.

## Product Name

The public product name is:

```text
Chaos Edition
```

Use `Chaos Edition` for user-visible product identity, release communication, and documentation that describes the fork as a product.

Do not use `S25 Chaos Edition` as the main visible name. It is too tied to one shorthand and does not clearly communicate the fork's relationship to Return to the Roots.

Do not use `RTTR Chaos Edition` or `Return to the Roots Chaos Edition` as the main visible name. Those names can imply official upstream status, endorsement, or ownership by the Return to the Roots project.

## Attribution Wording

Use this attribution wording when the relationship to upstream needs to be stated:

```text
Chaos Edition is an unofficial fork based on Return to the Roots.
```

This wording should be preferred in the launcher about dialog, README, release notes, package descriptions, and documentation.

References to `Return to the Roots` and `RTTR` are appropriate for:

- Attribution.
- Compatibility descriptions.
- Classic appearance descriptions.
- Explaining the upstream technical base.
- Explaining how Chaos Edition differs from upstream behavior.

Keep attribution factual. Do not write copy that suggests Chaos Edition is an official RTTR release, an upstream product line, or endorsed by upstream maintainers.

## Names to Avoid

Avoid these names as main visible product names:

- `S25 Chaos Edition`
- `RTTR Chaos Edition`
- `Return to the Roots Chaos Edition`
- `Official Chaos Edition`
- `Official RTTR Chaos Edition`
- `Return to the Roots: Chaos Edition`

Avoid wording that hides the fork boundary:

- `The new RTTR`
- `RTTR with Chaos mode`
- `Official Chaos build`
- `Return to the Roots fork by RTTR`

Use `Return to the Roots` or `RTTR` only when attribution, compatibility, or classic appearance context is needed.

## Internal Prefixes

Use the short internal prefix:

```text
chaos
```

Prefer `chaos` for settings, feature keys, metadata, data paths, and technical identifiers.

Good internal identifiers:

- `chaos.rulesProfile`
- `chaos.requiredFeatures`
- `chaos.theme`
- `chaos.compatibilityMode`
- `chaos.features`
- `chaos.assets`

Internal identifiers should be practical, stable, and easy to search. They should not carry marketing copy. Do not overuse `Chaos Edition` throughout the codebase when `chaos` is the clearer technical prefix.

Separate these concepts:

- Product name: `Chaos Edition`
- Attribution: `unofficial fork based on Return to the Roots`
- Technical prefix: `chaos`

## Where Branding Belongs

Concentrate branding at user-visible boundaries:

- Launcher title.
- About dialog.
- README and documentation.
- Release notes.
- Installer and package metadata.
- Map metadata where needed.
- Save metadata where needed.
- Error messages that explain Chaos-specific requirements.

Branding in these places helps users understand what they are running, where it came from, and whether a map or save needs Chaos-specific behavior.

## Release Identity Boundary v1

Chaos Edition currently separates visible product branding from release, build, and packaging identity.

Allowed and already established:

- Visible in-game and UI branding may say `Chaos Edition`.
- The about/options window may show the public product name and attribution.
- The main menu footer may show the public product name and attribution.
- The main menu or launcher area may show the public product name and attribution.
- Central edition metadata may expose `GetEditionName()`, `GetEditionAttribution()`, and `GetEditionInternalPrefix()`.

Current boundaries:

- `GetTitle()` remains unchanged for now.
- Binary, executable, package, installer, and CMake identity remain unchanged for now.
- App/window title identity remains a release decision, not a side effect of UI branding.
- No icon, asset, installer, package, binary, executable, or CMake naming change is implied by the current branding layer.
- Release artifact naming is not decided by this document's existing UI branding rules.

This split is intentional. Chaos Edition is its own product line, but `upstream/master` remains the technical base. Keeping visible UI branding separate from build and release identity lets the fork move in small steps, reduces build and packaging risk, and keeps upstream comparison easier. Collapsing all identity decisions into an incidental UI change would create avoidable review noise and make later rebases harder.

Release identity must be decided separately before a public release. Open decisions include:

- App/window title.
- Executable or binary name.
- Installer and package name.
- Config and save directory naming, if separate naming is needed.
- Icons and other identity assets.
- Release artifact naming.

Do not change release identity casually in gameplay, rules, compatibility, feature, addon, map, save, or balancing work. In particular, do not use a gameplay or rules PR to rename binaries, packages, installers, CMake targets, executable files, config directories, save directories, icons, assets, `GetTitle()`, or release artifacts. Those changes have different risk, validation, user-support, and upstream-sync costs, so they need their own scoped decision and review.

## Where Branding Does Not Belong

Do not spread product branding into every internal implementation detail.

Avoid `Chaos Edition` in:

- Generic helper names.
- Low-level utility functions.
- Build-system labels unrelated to packaging.
- Internal comments where `chaos` or a specific feature name is clearer.
- Data structures that only need a stable namespace.
- Repeated UI copy where the surrounding screen already establishes the product identity.

The point is not to erase the fork identity. The point is to avoid noisy names that make the codebase harder to maintain and later rename.

## Examples

Good user-visible naming:

```text
Chaos Edition
Chaos Edition is an unofficial fork based on Return to the Roots.
Classic RTTR-like appearance
RTTR-compatible rules profile
This save requires Chaos features that are not enabled by the selected rules profile.
```

Good technical identifiers:

```text
chaos.rulesProfile
chaos.requiredFeatures
chaos.theme
chaos.compatibilityMode
data/chaos/
```

Bad main product names:

```text
S25 Chaos Edition
RTTR Chaos Edition
Return to the Roots Chaos Edition
Return to the Roots: Chaos Edition
```

Bad technical identifiers:

```text
returnToTheRootsChaosEditionRulesProfile
rttrChaosEditionRequiredFeatures
s25ChaosEditionTheme
officialChaosMode
```

Better replacements:

```text
chaos.rulesProfile
chaos.requiredFeatures
chaos.theme
chaos.compatibilityMode
```

Naming discipline matters because bad naming creates false promises. If the product name implies official RTTR status, the project inherits confusion it does not own. If internal identifiers carry marketing text, the code gets harder to search, harder to rename, and easier to misuse. Keep the product name, attribution wording, and technical identifiers separate.
