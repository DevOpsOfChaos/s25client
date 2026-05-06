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
- Developer Preview: Ingame UI Workbench
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

## Developer-Only Ingame UI Preview Workbench And HUD Data Export

The ingame UI preview workbench and HUD data export are local developer review surfaces only. They are not normal
product UI and must not be described as player-facing features.

Use them for different jobs:

- Static Workbench: design sandbox with mock data. It does not load a map or read simulation state.
- HUD Data Export: the shared ViewModel contract that lists every planned compact HUD value group, including whether a
  value is mock data, placeholder data, live read-only data, or not safely accessible yet.

The old broad mock-state list was reduced. The current review direction is:

`modernized classic toggle-panel UI`

The workbench goal is to test whether the old Main Menu / submenu friction can be replaced by compact toggle buttons
that open smaller modernized panels, while still feeling like Settlers / RTTR rather than a foreign command center.

Start it from a devtools build:

```powershell
cmake --build .\build-vs-x64-debug-local --config Debug --target s25client
.\build-vs-x64-debug-local\Debug\s25client.exe
```

Navigation:

1. Open the main menu.
2. Double-click the version text in the lower-left corner to open the internal developer test screen.
3. Click `Ingame UI preview`, or press `i` on the developer test screen.
4. Switch through the reduced preview states with keys `1`-`6`, the on-screen developer state buttons, or
   `Tab`/`Shift+Tab`.
5. Press `Esc` or click `Back` to return to the developer test screen.

Available preview states:

- `Classic-inspired Compact HUD`: the main direction state. It shows narrow top/bottom bars, small resource and military
  chips, short message status, and persistent toggle buttons without recreating the old bulky lower menu wall.
- `Toggle Panels: Build/Roads`: compact Build and Roads panels with house, road, flag, mine, store, cancel, active, and
  disabled states. It uses the commands group but never dispatches commands.
- `Toggle Panels: Military/Economy`: compact soldier readiness/capacity and economy/resource rows using military,
  resources, and economy contract groups.
- `Messages + Minimap`: compact unread/latest-message panel, mute preview, toast preview, and static minimap placement
  comparison with zoom/collapse controls.
- `Selected Object / Context Panel`: compact selected-object panel with short context actions and resource snippets,
  driven by the selection and commands groups.
- `Small-screen Stress`: reduced pressure view for compact bars, toggles, messages, minimap, and context density at a
  1280x720-style safe area without changing real resolution.

Preview keyboard shortcuts:

- `1`: Classic-inspired Compact HUD
- `2`: Toggle Panels: Build/Roads
- `3`: Toggle Panels: Military/Economy
- `4`: Messages + Minimap
- `5`: Selected Object / Context Panel
- `6`: Small-screen Stress
- `Tab` / `Shift+Tab`: next / previous preview state

The workbench shows a static full-page ingame UI shell:

- fake terrain/checker background for density and contrast review
- narrow top and bottom HUD bars
- compact toggle buttons for Build, Roads, Military, Economy, Messages, Map, and Selection
- modernized classic-style panels with lighter frame weight and short labels
- representative resource, soldier, message, map, selection, command, and economy contract usage
- simulated minimap placeholder cards
- current preview size, GUI scale, texture filtering, and active preview state labels
- compact HUD data source, chip, resource, selection, map, and message diagnostics from `MockDeveloperHudDataProvider`
- HUD export contract diagnostics for player, military, resources, messages, selection, map/minimap, command rail, and
  economy groups
- explicit developer-only labels such as `Developer Preview: Ingame UI Workbench` and
  `Developer Preview | Static mock data | Not product UI | No gameplay logic`

It deliberately does not load a map, start simulation, open network/replay/save logic, use addons, or depend on
`GlobalGameSettings`.

## Developer-Only HUD Data Export

The workbench now exposes a HUD data export contract for the development UI. The point is to design against the values
the game HUD would need, without turning the normal ingame screen into the test surface.

Current export groups:

- `player`: active player id, active player name, compact player label
- `military`: total soldiers, rank breakdown, armored soldiers, military status, military capacity used/max
- `resources`: gold, coins, swords, food
- `messages`: unread message count, latest message label, compact message lane text
- `selection`: selected map point, selected object summary, future context actions
- `map`: map size, minimap thumbnail slot, viewport slot
- `commands`: command rail categories, build/road quick actions, disabled command dispatch marker
- `economy`: storage pressure and production alert slots

Availability states:

- `mock`: stable workbench data from `MockDeveloperHudDataProvider`
- `live read-only`: values that can be populated from safe game accessors in a future live adapter
- `placeholder`: deliberate UI contract slot with no game dependency yet
- `not safely accessible yet`: desired HUD value that needs a future accessor instead of private-structure digging

Values not safely accessible yet should get explicit future accessors instead of private-structure digging:

- latest message text without coupling to post internals
- real minimap thumbnail rendered through the existing minimap path
- current viewport/minimap focus rectangle
- military capacity/readiness summary across military buildings
- selected building detail ViewModel for workers, productivity, wares, and available actions
- storage pressure and production alerts for compact economy chips

When reviewing ingame UI changes in this workbench, check:

- `1280x720` windowed
- `1920x1080` windowed and fullscreen
- GUI scale default
- GUI scale high from Options -> Graphics, then re-enter the workbench
- texture filtering `Pixel / sharp`
- texture filtering `Smooth`
- clipped text
- overlapping controls or labels
- spacing and UI density
- contrast against the fake game background
- whether the result still reads as old pixel-art strategy UI rather than generic HD graphics
- whether anything looks too modern, too old, mobile-like, vector-like, futuristic, or realistic
- whether it still feels like Settlers / RTTR while being cleaner and faster to operate
- whether the compact toggle panels are better than old lower Main Menu / submenu navigation
- whether the top and bottom bars are small enough
- whether values are readable but not dominant
- which buttons belong permanently in the bar
- which panels should only open on demand
- whether the minimap belongs bottom-left, top-left, docked, toggled, or collapsible
- how large the minimap may become before it steals too much map-view space
- whether the notification toast and compact message panel feel helpful or interruptive
- whether the Mute/Stumm control is enough
- which soldier, resource, and economy numbers are genuinely important enough for persistent HUD space
- in the HUD data export, whether every value needed for the desired GUI has an explicit field
- whether mock/placeholder/not-safely-accessible statuses are honest enough to guide future accessor work
- whether Soldier/resource chips help decision-making or are just numeric noise
- whether the direction is too modern, too old, or exactly between

Real product UI changes still need user visual approval. The workbench only makes that approval cheaper; it does not
replace it. Do not wire the workbench into normal options, `Settings::video`, `CONFIG.INI`, addons, feature keys,
compatibility metadata, assets, savegames, replays, network state, or gameplay logic.

Feedback protocol for this workbench:

- State the preview state, resolution, display mode, GUI scale, and texture filtering mode.
- Call out exact clipped text, overlap, cramped density, weak contrast, or style drift.
- Answer these design questions explicitly:
  - Does it still feel like Settlers / RTTR?
  - Is it modernized, but not foreign?
  - Are the toggle panels better than old submenus?
  - Are the bars small enough?
  - Are values readable, but not dominant?
  - Which buttons belong permanently in the bar?
  - Which panels should only open on demand?
  - Where should the minimap sit, and should it be docked, toggled, or collapsible?
  - How large may the minimap be?
  - Is the notification toast or message panel disruptive?
  - Is Mute/Stumm enough?
  - Which soldier/resource/economy facts matter at a glance?
  - Are real soldier/resource/message values readable at HUD-chip size?
  - Is the direction too modern, too old, or exactly between?
- Distinguish "developer workbench problem" from "future product UI modernization requirement".
- Keep feedback grounded in the old strategy-game / modernized pixel-art target: clearer, denser where useful, but not
  foreign to the game.
