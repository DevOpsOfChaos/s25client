# Chaos Edition Feature Requirements

Chaos Edition is an unofficial fork based on Return to the Roots. This document is the developer and content-author catalog for `.chaos` feature requirements.

Feature requirements do not change gameplay by themselves. They describe the Chaos-specific capabilities that a map or save needs so the selected rules profile can accept or reject that content deterministically before it is used.

## Required Features

`.chaos` sidecar metadata may declare `requiredFeatures` as a comma-separated list of stable feature keys. Each key names a capability that must be available under the selected rules profile.

Use `requiredFeatures` when content depends on Chaos-only behavior, metadata handling, UI compatibility state, rules defaults, or future fork-specific content assumptions. Do not rely on filenames, map folders, visual appearance, or player intent to imply compatibility.

Existing maps and saves without `.chaos` sidecar metadata remain compatible with normal loading behavior. Missing sidecar metadata is treated as normal existing content, not as Chaos-only content.

Chaos-only content must declare `requiredFeatures` when it requires Chaos behavior. If it does not, the loader cannot distinguish intentional Chaos content from normal existing content, and the content author has created an ambiguous compatibility contract.

Unknown feature keys are rejected deterministically. They are not ignored, guessed, downgraded, or treated as optional.

## Rules Profiles

Chaos Edition currently defines two high-level rules profiles:

| Profile | Serialized value | Purpose |
| --- | --- | --- |
| RTTR-compatible | `rttr-compatible` | Behavior intended to remain compatible with upstream RTTR expectations where practical. This profile does not support Chaos-only feature keys. |
| Chaos | `chaos` | Chaos Edition behavior exposed through explicit feature gates, settings, addons, metadata, or dedicated data paths. |

The selected rules profile, not the launcher theme or visual appearance, decides compatibility. A classic-looking UI can still be running Chaos rules, and a Chaos launcher can still select RTTR-compatible behavior.

## Rules Profile Defaults

The Chaos profile currently changes only the default state of selected rules addons. RTTR-compatible keeps the old defaults so normal existing content does not silently opt into Chaos behavior.

| Behavior | Feature key | RTTR-compatible default | Chaos default | Can explicit config override? |
| --- | --- | --- | --- | --- |
| Tool ordering | `chaos.rules.tool_ordering_default_enabled` | off | on | yes |
| Automatic flag placement | `chaos.rules.automatic_flag_placement_default_enabled` | off | on | yes |

These feature keys describe registered and supported Chaos profile defaults. They are not reserved or planned keys. Content may require them when it depends on those defaults being available under the selected profile.

Saved or explicitly configured addon values override profile defaults. A profile default is only the initial behavior when no stored or explicit addon configuration has already chosen a value.

`requiredFeatures` does not turn an addon on. It describes compatibility requirements so the loader can reject content whose assumptions are unsupported by the selected rules profile. Chaos-only content that relies on the Chaos defaults above should declare the matching `requiredFeatures` entries instead of assuming the selected profile, UI theme, or file location proves compatibility.

## Known Feature Keys

The authoritative in-code catalog is `chaos::KnownFeatureDefinitions` in `libs/s25main/ChaosFeatureRequirements.h`. Use `chaos-metadata features` to print the current runtime view.

| Feature key | Category | User-facing | RTTR-compatible | Chaos | Notes |
| --- | --- | --- | --- | --- | --- |
| `chaos.rules_profile` | `metadata` | yes | no | yes | Requires Chaos rules profile metadata support. |
| `chaos.extended_content` | `reserved` | no | no | no | Reserved key. Known, but currently unsupported by every profile. |
| `chaos.extended_ai` | `reserved` | no | no | no | Reserved key. Known, but currently unsupported by every profile. |
| `chaos.extended_visuals` | `reserved` | no | no | no | Reserved key. Known, but currently unsupported by every profile. |
| `chaos.map_metadata_v1` | `reserved` | no | no | no | Reserved key. Known, but currently unsupported by every profile. |
| `chaos.ui.compatibility_preview_status` | `ui` | yes | no | yes | Requires Chaos compatibility preview status support. |
| `chaos.rules.tool_ordering_default_enabled` | `rules` | yes | no | yes | Requires the Chaos rules profile default that enables tool ordering. |
| `chaos.rules.automatic_flag_placement_default_enabled` | `rules` | yes | no | yes | Requires the Chaos rules profile default that enables automatic flag placement. |

RTTR-compatible currently supports no Chaos feature keys. Any `.chaos` sidecar that requires a Chaos-only key is incompatible with the RTTR-compatible profile.

Reserved keys are intentionally known but unsupported. They are useful for planned capability names, but content must not require them until a profile explicitly supports them.

Registered and supported rules keys describe behavior that is implemented by the selected profile. Reserved keys are only planned names and must stay separate from shipping content requirements until the in-code catalog marks a profile as supporting them.

## Sidecar Example

For content at `example.swd`, place metadata next to it as `example.swd.chaos`:

```text
rulesProfile=chaos
requiredFeatures=chaos.rules_profile,chaos.rules.tool_ordering_default_enabled
minChaosVersion=0.1.0
```

`rulesProfile` declares the required high-level profile. `requiredFeatures` declares mandatory capabilities under that profile. `minChaosVersion` is optional metadata for version communication.

Keep the sidecar format line-based and deterministic. Do not add ad hoc keys or alternate syntax unless the metadata format itself is deliberately versioned in a separate change.

## Authoring Commands

Use `chaos-metadata features` to inspect the known feature catalog:

```sh
chaos-metadata features
```

The command prints:

```text
key	category	supportedProfiles	userFacing
chaos.rules_profile	metadata	chaos	yes
...
```

Use `chaos-metadata create` to create a sidecar file for content:

```sh
chaos-metadata create example.swd --rules-profile chaos --required-features chaos.rules_profile,chaos.rules.tool_ordering_default_enabled
```

Add `--min-chaos-version <version>` when the content needs to communicate a minimum Chaos Edition version. Add `--overwrite` only when replacing an existing sidecar intentionally.

Use `chaos-metadata validate` to check a sidecar against a selected profile:

```sh
chaos-metadata validate example.swd --rules-profile chaos
chaos-metadata validate example.swd.chaos --rules-profile rttr-compatible
```

Validation returns structured text with `status`, `path`, and `message`. Invalid metadata exits as invalid. Unsupported requirements exit as incompatible. A successful result means the metadata is syntactically valid and the selected profile provides all required features.

## Authoring Rules

- Existing maps and saves without `.chaos` metadata remain compatible.
- Chaos-only content must declare required features when it depends on Chaos behavior.
- RTTR-compatible content must not require Chaos-only feature keys.
- Unknown feature keys are rejected deterministically.
- Known but unsupported reserved keys must not be required by shipping content.
- Do not use `.chaos` metadata to silently reinterpret normal RTTR-compatible content as Chaos-only content.
