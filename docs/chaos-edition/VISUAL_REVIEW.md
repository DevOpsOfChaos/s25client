# Chaos Visual Review Workflow

This workflow applies to Chaos Edition presentation prototypes. The goal is to keep visual changes reviewable before they become persistent product behavior.

Chaos Edition targets modernized pixel-art presentation, not generic HD graphics. Tests can prove math and regressions, but they cannot prove that the result still looks like the intended old-strategy-game presentation.

## Stop Point

Codex must stop for manual visual review after any block that visibly changes UI or rendering. This includes:

- texture filtering, sampler state, texture upload, or texture packing behavior
- viewport, projection, render-size, GUI-scale, DPI, fullscreen, or window-resize behavior
- render-to-texture, final blit, letterboxing, or screenshot readback behavior
- cursor, hover, hit-test, mouse mapping, selection rectangle, minimap, overlay, or road-building presentation
- menu, options, map-selection, loading, ingame frame, or window/control layout changes
- any developer-only preview screen that draws new presentation diagnostics

Green automated tests are required where applicable, but green tests are not visual approval.

Codex may build and run local prototypes. Codex must not commit visible UI or rendering changes until either:

- the user explicitly approves the visual result, or
- the change is explicitly non-visible, disabled by default, or developer-only.

If the user asks for visual adjustments, Codex applies them, rebuilds, reruns the relevant checks, opens the program when possible, and repeats the visual review step.

## Local Commands

Configure the local Visual Studio build if the build folder does not exist:

```powershell
cmake -S . -B .\build-vs-x64-debug-local -G "Visual Studio 17 2022" -A x64 -DRTTR_INCLUDE_DEVTOOLS=ON -DBOOST_ROOT="C:\dev\deps\boost_1_87_0-msvc-14.3-64" -DBOOST_INCLUDEDIR="C:\dev\deps\boost_1_87_0-msvc-14.3-64"
```

Build the client:

```powershell
cmake --build .\build-vs-x64-debug-local --config Debug --target s25client
```

Start the client:

```powershell
.\build-vs-x64-debug-local\Debug\s25client.exe
```

Run the focused helper test target when presentation helper code changes:

```powershell
cmake --build .\build-vs-x64-debug-local --config Debug --target Test_simple
ctest --test-dir .\build-vs-x64-debug-local -C Debug -R "^Test_simple$" --output-on-failure
```

Run UI or driver tests when UI, viewport, input, or driver behavior changes:

```powershell
cmake --build .\build-vs-x64-debug-local --config Debug --target Test_UI
ctest --test-dir .\build-vs-x64-debug-local -C Debug -R "^Test_UI$" --output-on-failure
cmake --build .\build-vs-x64-debug-local --config Debug --target Test_drivers
ctest --test-dir .\build-vs-x64-debug-local -C Debug -R "^Test_drivers$" --output-on-failure
```

The executable logs the resolved user-data directory on startup. Local `CONFIG.INI`, `IngameOptions.ini`, logs, saves, and screenshots live below that user-data root. Screenshots are written below `<RTTR_USERDATA>\screenshots`.

## Manual Test Matrix

For the next visible presentation/scaling prototype, review at least these screens and flows:

- Main menu
- Options -> Graphics
- Map selection
- In-game map view
- mouse hover and click alignment on menu controls and ingame UI
- road building and selection rectangle, if quickly reachable
- minimap and overlays, if quickly reachable
- screenshot behavior when the prototype affects viewport, final blit, or letterboxing

Review these display states:

- `1280x720` windowed
- `1920x1080` windowed and fullscreen
- `2560x1440`, if available
- `3840x2160`, if available
- GUI scale default or automatic
- GUI scale high fixed value
- texture filtering `Pixel / sharp`
- texture filtering `Smooth`

For presentation-scaling work, record the observed source size, target size, integer scale, viewport, margins, and whether mapping is active or using fallback. A screenshot is useful evidence, but do not replace interactive review with screenshot comparison tests.

## Review Steps For User Feedback

When Codex reaches a stop point, the handoff should include:

- exact commit or working-tree state being reviewed
- exact build command that succeeded
- exact command used to start the executable
- screens and flows to inspect from the manual matrix
- what visual behavior changed and what should stay unchanged
- any known fallback, disabled, or developer-only limitation

Feedback should be concrete: resolution, display mode, GUI scale, filtering mode, screen/flow, expected result, observed result. Codex should turn that feedback into the smallest follow-up fix and repeat the review loop.

Commit only after visual approval or after confirming the change is non-visible, disabled, or developer-only.

## Developer-Only Preview Concept

A future presentation preview can be useful, but it must not be sold as normal product UI. It should be a local review/debug surface only.

The repository already has an internal developer test screen reachable by double-clicking the version text on the main menu, plus a texture test screen. That is a plausible future entry point for a presentation preview, but this workflow does not add one because a new screen would still be visible UI work and would require its own review.

A future preview should show:

- side-by-side scaling/filtering states
- checkerboard background, letterbox bars, sample sprite, sample text, and sample UI control
- source size and target size
- integer scale
- viewport rectangle
- top, right, bottom, and left margins
- mapping active or fallback
- texture filtering state and final-blit sampler state

Do not wire this into normal options, `Settings::video`, `CONFIG.INI`, addons, feature keys, compatibility metadata, assets, savegames, replays, network state, or gameplay logic.
