# HD Graphics / Rendering Audit v1

See [Visual Modernization Direction](VISUAL_DIRECTION.md) for the style boundary: Chaos Edition is pursuing modernized pixel-art presentation, not generic HD graphics.

## Executive summary

Chaos Edition must keep HD graphics work local to presentation. It must not be modeled as a gameplay addon, because addons are part of `GlobalGameSettings`, are serialized, and are exchanged through network/replay/game setup paths. A graphics option that changes savegame content, map interpretation, global game settings, or command generation would be the wrong boundary.

The current code already has a useful separation:

- `libsiedler2::Archiv` and `Loader` resolve and cache original S2 and RTTR resources.
- `GlAllocator` maps loaded archive items to OpenGL-capable item classes.
- `glArchivItem_*`, `glSmartBitmap`, `glTexturePacker`, and `TerrainRenderer` create and draw textures.
- `Settings::video`, `dskOptions`, `iwSettings`, `VideoDriver`, and `VideoDriverWrapper` own local video/window/UI scaling state.
- `GameWorldView` owns local ingame zoom and view transforms.

The safest first implementation block is a local texture filtering mode applied at texture creation/upload time. Chaos Edition v1 implements that as a local `Settings::video.textureFiltering` display option. Settings-controlled texture uploads use `VideoDriverWrapper::SetConfiguredTextureFilter()`, which delegates sampler selection to the central `TextureFilter` helper.

## Explicit non-goals for v1 audit and first implementation

- Do not implement HD graphics as an addon.
- Do not add or replace assets.
- Do not change map, savegame, replay, network, or simulation formats.
- Do not change `GlobalGameSettings`, game messages, replay metadata, map loading semantics, or addon serialization.
- Do not change CMake, packaging, binary identity, or executable branding as part of a first rendering option.
- Do not build an asset override system before exhausting the existing resource override model.

## Asset loading

### Current loading model

The resource model is documented in `doc/ResourceLoading.md`. The key structure is `libsiedler2::Archiv`: resources are treated as archives, and archive entries are addressed by a resource id plus an index. A `ResourceId` is the lower-case filename without extension, so `FOO.LST`, `foo.bob`, and a `foo` folder can resolve to the same logical resource.

There are two major load phases:

- Program start: GUI, palettes, fonts, loading screens, sounds, menu resources.
- Game start: map-specific graphics, nation graphics, terrain/map graphics, BOB/job/carrier resources, and addon-dependent resources.

### Relevant files/classes/functions

- `doc/ResourceLoading.md`
  - Explains the layered archive and override model.
- `libs/rttrConfig/src/files.h`
  - Defines original S2 paths such as `<RTTR_GAME>/DATA`, `<RTTR_GAME>/GFX/PICS`, `<RTTR_GAME>/GFX/TEXTURES`, `<RTTR_GAME>/DATA/MBOB`, and RTTR asset folders.
- `libs/s25main/resources/ResourceId.{h,cpp}`
  - Converts file paths to extensionless lower-case resource ids.
- `libs/s25main/resources/ArchiveLocator.{h,cpp}`
  - Resolves a base asset plus layered override files/folders.
- `libs/s25main/resources/ArchiveLoader.{h,cpp}`
  - Loads resolved files/folders through `libsiedler2::Load()` / `LoadFolder()` and merges archives.
  - Handles BOB override merging via `glArchivItem_Bob::mergeLinks()`.
- `libs/s25main/Loader.{h,cpp}`
  - Central cache and typed access point for resources.
  - `LoadFilesAtStart()` loads palettes, fonts, GUI, setup/load screens, sounds, and RTTR UI resources.
  - `LoadFilesAtGame()` loads map/nation/game resources.
  - `fillCaches()` builds `glSmartBitmap` caches for buildings, flags, workers, carriers, animals, trees, granite, grainfields, boats, donkeys, gateway, and fight animations.
- `libs/s25client/s25client.cpp`
  - Installs `GlAllocator` via `libsiedler2::setAllocator(new GlAllocator())` before game startup.
- `external/libsiedler2/include/libsiedler2/libsiedler2.h`
  - Public load/write entry points.
- `external/libsiedler2/src/libsiedler2.cpp`
  - Extension-based format dispatch.
- `external/libsiedler2/src/LoadType.cpp`
  - Maps archive entry `BobType` to concrete archive item classes.

### Relevant formats/containers

Graphics-relevant formats currently dispatched by `libsiedler2::Load()` include:

- `DAT` / `IDX`: original archive pairs, including `RESOURCE.DAT` and `IO.DAT`.
- `LST`: archive container.
- `BOB`: body/overlay animation container.
- `BMP`: bitmap image.
- `LBM`: loading/menu/background image, including palette animation support.
- `BBM` / `ACT`: palettes.
- Folder archives: user/RTTR override folders parsed as archive entries.
- `SWD` / `WLD`: maps. These are not graphics assets and must not be touched for a first HD/scaling option.

Sound/text/config formats also pass through the same archive system, but they are outside this audit's first rendering path.

### Current asset layering

`addDefaultResourceFolders()` in `Loader.cpp` adds resources in this order:

1. `assets/base`
2. nation-specific override folder
3. `assets/overrides`
4. enabled addon asset folders
5. user overrides under `<RTTR_USERDATA>/LSTS`

This means Chaos Edition already has an override mechanism. Adding another asset override lookup path early would duplicate an existing concept and create compatibility risk unless the exact precedence and fallback rules are designed first.

## Rendering pipeline

### High-level flow

1. `GameManager::Run()` drives the frame loop.
2. `VideoDriverWrapper::ClearScreen()` clears the OpenGL buffer.
3. `WindowManager::Draw()` draws the active desktop and ingame windows.
4. `Desktop::Draw_()` draws optional full-screen background images.
5. `dskGameInterface::Run()` calls `GameWorldView::Draw()` for the ingame world.
6. `GameWorldView::Draw()` applies view/zoom transforms, draws terrain through `TerrainRenderer`, then draws objects, figures, overlays, catapult stones, and local GUI elements.
7. `WindowManager` draws ingame windows, tooltips, and the cursor after the desktop/game view.
8. `VideoDriverWrapper::SwapBuffers()` presents the frame.

### Renderer abstractions

- `libs/s25main/ogl/IRenderer.h`
  - Minimal renderer abstraction for basic UI primitives: 3D borders/content, rectangles, and lines.
- `libs/s25main/ogl/OpenGLRenderer.{h,cpp}`
  - OpenGL implementation of `IRenderer`.
- `libs/s25main/ogl/DummyRenderer.{h,cpp}`
  - Non-OpenGL/test fallback.
- `libs/s25main/ogl/ITexture.h`
  - Common texture draw interface used by UI controls and renderable assets.
- `libs/s25main/drivers/VideoDriverWrapper.{h,cpp}`
  - Owns renderer selection, viewport/projection setup, texture handle tracking, frame limiting, buffer swapping, and driver delegation.

The abstraction is not a modern renderer abstraction. Most drawing still uses direct OpenGL state and immediate/client-array style calls inside `glArchivItem_*`, `glSmartBitmap`, `TerrainRenderer`, and UI code. A first option should therefore avoid broad renderer refactors.

### Texture creation and drawing

- `libs/s25main/ogl/glArchivItem_BitmapBase.{h,cpp}`
  - Owns per-bitmap OpenGL texture handle.
  - `GetTexture()` lazily creates textures.
  - `GenerateTexture()` chooses texture size, binds the texture, sets `GL_TEXTURE_MIN_FILTER` / `GL_TEXTURE_MAG_FILTER`, then calls `FillTexture()`.
  - `setInterpolateTexture(bool)` invalidates the texture so it regenerates with a new filter.
- `libs/s25main/ogl/glArchivItem_Bitmap.{h,cpp}`
  - Draws normal bitmap textures using vertex/texcoord arrays.
  - `DrawFull()` and `Draw()` are common sprite draw entry points.
- `libs/s25main/ogl/glArchivItem_Bitmap_Player.{h,cpp}`
  - Handles player-color variants.
- `libs/s25main/ogl/glSmartBitmap.{h,cpp}`
  - Composes multiple archive bitmaps into one draw unit for buildings, units, shadows, and player colors.
  - `generateTexture()` uploads composed BGRA buffers and hard-codes nearest filtering.
- `libs/s25main/ogl/glTexturePacker.{h,cpp}`
  - Packs `glSmartBitmap` instances into shared textures when `SETTINGS.video.sharedTextures` is enabled.
  - `glTexture` hard-codes nearest filtering.
- `libs/s25main/TerrainRenderer.{h,cpp}`
  - Builds terrain, edge, and road textures from game data descriptions and renders terrain through OpenGL triangles/VBOs or client arrays.

### UI vs ingame rendering split

- UI shell:
  - `WindowManager`, `Window`, `Desktop`, `IngameWindow`, controls under `libs/s25main/controls`.
  - `Window::Draw3D*()` uses `IRenderer`.
  - `ctrlBaseImage`, `ctrlImage`, `ctrlImageButton`, and `ctrlImageDeepening` use `ITexture`.
- Ingame world:
  - `dskGameInterface` owns the game desktop and frame border/button bar.
  - `GameWorldView` owns view-space drawing, zoom, mouse-to-map transforms, overlays, and object drawing order.
  - `TerrainRenderer` owns terrain geometry/texture batching.
  - `nodeObjs`, `figures`, and `buildings` generally draw themselves with `LOADER` texture accessors.

This split is good enough for a local rendering option, but not clean enough for a large HD refactor without invasive work.

### Backends

- `libs/driver/include/driver/VideoInterface.h`
  - The video driver interface.
- `libs/driver/src/VideoDriver.cpp`
  - Base video driver state, DPI scale, GUI scale, render size, and mouse coordinate conversion.
- `extras/videoDrivers/SDL2/VideoSDL2.{h,cpp}`
  - SDL2 OpenGL backend.
- `extras/videoDrivers/WinAPI/WinAPI.{h,cpp}`
  - WinAPI OpenGL backend.

`VideoDriverWrapper` chooses an `OpenGLRenderer` when the driver reports OpenGL support. Both real backends are OpenGL-backed from the rendering perspective.

## Scaling and zoom

### Existing GUI scale

- `libs/driver/include/driver/GuiScale.h`
  - Converts between view and screen coordinates.
- `libs/driver/src/VideoDriver.cpp`
  - `SetNewSize()` computes DPI scale and `scaledRenderSize_`.
  - Stores both the physical render size and the GUI-scaled render size.
  - `setGuiScalePercent()` changes local GUI scaling and triggers resize/mouse updates.
  - `getGuiScaleRange()` computes min/max/recommended GUI scale.
- `libs/s25main/drivers/VideoDriverWrapper.cpp`
  - `RenewViewport()` uses render size for `glOrtho()`.
- `libs/s25main/desktops/dskOptions.cpp`
  - Main menu graphics tab exposes GUI scale.
- `libs/s25main/ingameWindows/iwSettings.cpp`
  - Ingame settings expose display/window options, not GUI scale.

`SETTINGS.video.guiScale` is local and persistent. It is not part of `GlobalGameSettings`.

The important boundary is that GUI scale changes the UI/view coordinate space reported by `GetRenderSize()`. It is not world zoom. It changes how menu controls, windows, desktop surfaces, and ingame UI are laid out and hit-tested.

### Existing ingame zoom

- `libs/s25main/desktops/dskGameInterface.cpp`
  - Defines zoom stepping via `ZOOM_FACTORS`, wheel zoom, and keyboard zoom shortcuts.
  - `Msg_KeyDown()` handles keyboard zoom reset/in/out.
  - `Msg_WheelUp()`, `Msg_WheelDown()`, and `WheelZoom()` handle wheel zoom.
  - `Msg_MouseMove()` uses `GameWorldView::ViewPosToMap()` for grab-and-drag scrolling.
- `libs/s25main/world/GameWorldView.{h,cpp}`
  - `zoomFactor_`, `targetZoomFactor_`, `effectiveZoomFactor_`, and `zoomSpeed_`.
  - `SetZoomFactor()`, `SetNextZoomFactor()`, `ViewPosToMap()`, `CalcFxLx()`, and `Draw()` handle local view zoom.
  - `Draw()` applies `glScalef()` and `glTranslatef()` to the projection/model-view path.
  - `updateEffectiveZoomFactor()` partially adjusts zoom using DPI/GUI scale.

This is local view state. It should still be treated carefully because mouse-to-map conversion and command targeting depend on it.

World zoom is therefore inside `dskGameInterface` and `GameWorldView`, not in `Settings::video`. It is a local presentation transform, but it feeds local command targeting through `selPt`, context clicks, road building, scrolling, and action-window placement.

### Window, render, viewport, and input coupling

Current coupling:

- Backend drivers receive OS/window events and call `VideoDriver::SetNewSize(windowSize, renderSize)`.
- `VideoDriver::SetNewSize()` computes `dpiScale_`, optionally recomputes automatic GUI scale, then stores `scaledRenderSize_ = guiScale_.screenToView(renderSize_)`.
- `VideoDriverWrapper::GetRenderSize()` returns that GUI-scaled render size, not the raw framebuffer size.
- `VideoDriverWrapper::GetWindowSize()` returns at least the minimum logical window size and is used by viewport/scissor setup.
- `VideoDriverWrapper::RenewViewport()` sets `glViewport()` and `glScissor()` to the whole window, then sets an orthographic projection from `(0, 0)` to `GetRenderSize()`.
- `WindowManager::Msg_ScreenResize()` and desktop/window resize handlers consume `GetRenderSize()`.
- `VideoDriver::setGuiScalePercent()` converts the current mouse position through `GuiScale` before and after the scale change, then sends a fresh mouse-move event.
- Mouse events are already in view coordinates when game/UI code sees `MouseCoords`.
- `GameWorldView::Draw()` converts view coordinates to a scissored world area and separately adjusts mouse position when `effectiveZoomFactor_ != 1`.
- `GameWorldView::ViewPosToMap()` mirrors the world zoom transform for map scrolling and selection logic.

The current pipeline assumes one full-window viewport and one orthographic view coordinate system. An integer-scaled sub-viewport would break that assumption unless both drawing and input are transformed through the same presentation rectangle.

### Texture filtering v1 coverage

Chaos Edition has a user-facing local pixel/smooth texture filtering option. It is a display preference only, not gameplay configuration.

Covered settings-controlled paths:

- `glArchivItem_BitmapBase::GenerateTexture()` for regular archive bitmap textures that follow the video setting.
- `glSmartBitmap::generateTexture()` for generated smart bitmap textures.
- `glTexture` in `glTexturePacker.cpp` for packed shared textures.
- `VideoDriverWrapper::SetConfiguredTextureFilter()` and `UpdateConfiguredTextureFilters()` for applying and refreshing the current local setting.

Central mapping:

- `Pixel / sharp`: `GL_NEAREST`
- `Smooth`: `GL_LINEAR`

Pixel / sharp remains the default because the source art is pixel art and hard edges are the conservative presentation baseline. Smooth remains opt-in because it trades crispness for interpolation and should reflect only the local display preference of the player.

Explicit legacy/special paths that call `setInterpolateTexture(false)` remain smooth by design. Known examples include splash and minimap setup paths. These do not become settings-controlled paths in v1 because changing them would alter existing explicit presentation behavior rather than harden coverage.

### Integer scaling / pixel-perfect status

No explicit integer scaling or pixel-perfect viewport policy exists. The current system has:

- GUI scaling in percentages.
- DPI-corrected ingame zoom.
- Arbitrary OpenGL projection and zoom transforms.
- Power-of-two texture sizing via `VideoDriverWrapper::calcPreferredTextureSize()`.

Integer scaling is therefore not a small texture-filter toggle. A real implementation likely needs either viewport letterboxing, a render-to-texture presentation pass, or constrained zoom/scale transforms. That touches mouse coordinate mapping and screen-size behavior, so it should not be the first block.

Chaos Edition now has an isolated calculation helper for future experiments:

- `libs/s25main/helpers/IntegerScaling.{h,cpp}`
  - `helpers::CalculateIntegerScaleViewport(sourceSize, targetSize)` returns the largest integer scale that fits and a centered viewport rectangle.
  - The returned viewport follows the existing `Rect` convention: left/top are inside, right/bottom are outside.
  - Centering is deterministic. Even leftovers split evenly; odd leftovers keep the extra pixel on the right/bottom side because the origin uses integer floor division.
  - The helper rejects zero source or target dimensions with a deterministic empty, non-fitting result.
  - If the target is too small for 1x, it returns scale `1`, marks `fits = false`, and uses the available target rectangle as a deterministic fallback when that rectangle is representable.
  - Oversized dimensions that cannot be represented by the current signed `Rect` type are rejected with a deterministic empty, non-fitting result rather than relying on overflowing conversions.
  - `helpers::IsTargetPointInsideIntegerScaleViewport()` checks whether a target/window point is inside the integer presentation rectangle and deliberately treats letterbox areas as outside.
  - `helpers::MapTargetPointToSourcePoint()` maps a target/window point to source/presentation coordinates only when the point is inside a fitting viewport.
  - `helpers::MapSourcePointToTargetPoint()` maps a source/presentation point to the target/window top-left pixel using the same origin and scale contract, and rejects source points on or beyond the source right/bottom boundary.
  - `helpers::MapSourceRectToTargetRect()` maps source rectangles to target rectangles using the same origin and scale contract, and rejects rectangles outside the declared source size.
  - It has no GL calls, no settings persistence, no UI, no driver side effects, and no mouse-coordinate changes.
- `tests/s25Main/simple/testIntegerScaling.cpp`
  - Covers exact 1x, 2x/3x fitting, lower-integer selection for non-integer targets, even and odd centering, tiny-window fallback, zero dimensions, target-to-source mapping, source-to-target mapping, source-rectangle mapping, viewport boundary behavior, letterbox rejection, and reasonable large dimensions.

This helper is deliberately not an implementation of pixel-perfect scaling. It is only a tested place to harden the presentation math before touching viewport, render-to-texture, or input paths.

Future letterbox/input work should treat this helper as the single arithmetic contract:

- Draw/presentation code should use the calculated viewport rectangle for the final integer-scaled image placement.
- Input code should first reject points outside that viewport, then map target/window coordinates back to source/presentation coordinates through `MapTargetPointToSourcePoint()`.
- Source-driven overlays or debug rectangles should use the source-to-target helpers so their origin and scale match the presentation pass exactly.
- Fallback states with `fits = false` are not valid integer presentation states. They exist to make tiny or invalid dimensions deterministic, not to silently enable input mapping.

Still intentionally not implemented:

- No renderer integration.
- No render-to-texture presentation pass.
- No `VideoDriverWrapper::RenewViewport()` behavior change.
- No runtime mouse-coordinate conversion.
- No `GameWorldView`, `dskGameInterface`, or input-system changes.
- No `Settings::video.integerScaling`, UI option, `CONFIG.INI` persistence, addon, feature key, `.chaos` metadata, savegame, network, replay, asset, CMake, packaging, or binary-identity change.

The UI option remains premature because the hard part is not offering a checkbox. The hard part is proving that draw coordinates, window coordinates, source coordinates, GUI hit testing, world selection, road building, scrolling, and resize behavior all use the same presentation rectangle. Shipping a persistent option before that proof would create a user-visible compatibility promise around unfinished coordinate behavior.

Required future tests before renderer integration:

- Driver-level target-to-view and view-to-target translation with non-zero letterbox origins.
- UI hit tests for buttons, modal windows, dragging, snapping, cursor hover, and tooltip behavior inside a letterboxed viewport.
- `GameWorldView::ViewPosToMap()` coverage with default, minimum, and maximum zoom after presentation-coordinate mapping.
- Ingame interaction tests for context clicks, road building, grab-and-drag scrolling, action-window placement, minimap interaction, and resize events.
- Visual and screenshot checks for windowed/fullscreen, automatic/fixed GUI scale, pixel/smooth filtering, shared textures on/off, VBO on/off, and SDL2/WinAPI backends where available.

### Later option shape

A future local option should probably be:

- `Auto`
  - Conservative default once the implementation is proven. It may choose integer scaling when it fits cleanly and fall back to current behavior when it would create an unusable viewport.
- `Integer / pixel-perfect`
  - Forces integer presentation. It should expose letterboxing/clipping behavior clearly and must never alter simulation or synchronized game state.
- `Free / current behavior`
  - Keeps today's full-window orthographic behavior with arbitrary GUI scale and world zoom.

Do not add this option until viewport/input tests and manual visual checks exist. Adding a setting before the path is proven would only create a compatibility promise around unfinished behavior.

### Texture filtering interaction

Texture filtering and integer scaling solve different problems:

- Texture filtering chooses sampler behavior for texture uploads: nearest for `Pixel / sharp`, linear for `Smooth`.
- Integer scaling chooses how a rendered source area is presented inside a target window/framebuffer.
- `Pixel / sharp` plus integer scaling is the most faithful pixel-art path because it avoids both texture interpolation and fractional presentation scaling.
- `Smooth` plus integer scaling is still valid as a local preference, but it intentionally softens texture samples even when the final presentation scale is integer.
- `Smooth` does not make non-integer scaling pixel-perfect. It hides some artifacts but changes the look.
- `Pixel / sharp` without integer scaling can still shimmer or show uneven pixel sizes under fractional presentation transforms.

The first implementation should keep these as independent local video preferences unless testing proves that a combined preset is safer.

## Settings

### Local persistent settings

Local settings live in `Settings` and are persisted through `CONFIG.INI` and `IngameOptions.ini`:

- `Settings::video`
  - `fullscreenSize`
  - `windowedSize`
  - `framerate`
  - `displayMode`
  - `vbo`
  - `sharedTextures`
  - `guiScale`
  - `textureFiltering`
- `Settings::interface`
  - local interface behavior such as map scroll mode and window pinning.
- `Settings::ingame`
  - local HUD/window preferences such as building-quality overlay, names, productivity, minimap, and statistics scale.
- `Settings::windows`
  - local persistent ingame window placement/state.

Good future location for local rendering options:

- `Settings::video`, because texture filtering is presentation-only and belongs with `vbo`, `sharedTextures`, and `guiScale`.
- `dskOptions` graphics tab for the main menu UI.
- Possibly `iwSettings` later if the option must be changed while in a game.

### Settings that must not receive HD/scaling state

Do not put local graphics/scaling/filter options in:

- `GlobalGameSettings`
- `gameTypes/GameSettingTypes`
- addon definitions or addon state
- network game messages carrying `GlobalGameSettings`
- savegame settings blocks
- replay metadata/settings blocks
- map generator settings
- Lua game data settings

Those paths are synchronized, serialized, gameplay-facing, or compatibility-sensitive. Putting a local rendering preference there would create exactly the coupling Chaos Edition is trying to avoid.

## UI and fixed images

### Fixed UI resource loading

Main fixed UI assets are loaded in `Loader::LoadFilesAtStart()`:

- `resource`: original `RESOURCE.DAT`
- `io`: original `IO.DAT`
- `setup013`, `setup015`: original LBM background images
- load screens from `<RTTR_GAME>/GFX/PICS` and `<RTTR_GAME>/GFX/PICS/MISSION`
- RTTR resources: `io_new`, `client`, `languages`, `logo`, `menu`, `rttr`
- splash image: `s25::files::splash` is loaded separately in `GameManager::ShowSplashscreen()`

### UI draw entry points

- `Desktop::Draw_()`
  - Draws a full desktop background with `background->DrawFull(GetDrawRect())`.
- `Window::Draw3DBorder()` / `Draw3DContent()`
  - Uses `io` textures and `IRenderer`.
- `ctrlBaseImage::DrawImage()`
  - Centers/crops images inside destination rectangles and calls `ITexture::Draw()`.
- `ctrlImage`, `ctrlImageButton`, `ctrlImageDeepening`
  - Fixed image controls.
- `dskGameInterface::Msg_PaintBefore()`
  - Draws ingame frame statues, borders, and button bar from `resource`.
- `dskOptions`
  - Uses `setup013` as background and many `io` / portrait resources.

### UI/ingame asset separation

There is a practical separation, but not a hard package boundary:

- UI mostly uses `resource`, `io`, `io_new`, `menu`, `logo`, `rttr`, `setup*`, and control/window classes.
- Ingame sprites mostly use map/nation/BOB resources via `GetMapTexture`, `GetNationImage`, `GetPlayerImage`, `glSmartBitmap` caches, and `TerrainRenderer`.
- Some ingame windows use ingame icons and map textures for inventory/building/job/ware UI.

Small future UI polish should target fixed UI surfaces first:

- splash/load/menu backgrounds
- desktop backgrounds in `Desktop`
- main menu/options graphics tab controls
- `ctrlBaseImage` sizing/cropping behavior
- static button/icon image use in `dskGameInterface`

Avoid replacing original UI assets in early work.

## Risk assessment

### Safe local-only areas

- `Settings::video` for a local filtering enum/setting.
- `dskOptions` graphics tab for a local UI control.
- `iwSettings` only if ingame adjustment is required.
- `VideoDriverWrapper` OpenGL state around texture binding/filter application.
- `glArchivItem_BitmapBase`, `glSmartBitmap`, and `glTexturePacker` for texture filtering if changes are limited to OpenGL sampler state.
- `GameWorldView` view zoom only for presentation behavior, with mouse mapping tests.
- UI controls that draw `ITexture` without changing resource ids or game data.
- Pure helper code that computes integer scale and viewport rectangles from source/target dimensions.
- A later local presentation pass that maps a rendered image into a viewport, if mouse/event coordinates are transformed through the same rectangle.
- Letterbox clear color and presentation-only viewport/scissor setup, if isolated from world simulation and resource loading.

### Risky synchronized/gameplay areas

- `GlobalGameSettings` and all addon registration/state.
- `network/GameMessages`, `GameClient`, `GameServer`, lobby settings exchange.
- Savegame and replay load/save paths.
- `GameWorldBase`, `GameWorld`, `MapNode`, pathfinding, events, random generation, and map geometry.
- `TerrainRenderer` terrain selection, vertex generation based on world state, visibility, altitude, and road data.
- Lua gamedata and `WorldDescription` content if used to alter terrain/object definitions.
- `ArchiveLoader` / `ArchiveLocator` behavior if changing resource precedence or allowing new fallback directories.
- Mouse-coordinate conversion in backend drivers or `WindowManager` unless covered by tests for UI hit testing, world selection, road building, scrolling, and window dragging.
- Changing `GameWorldView::ViewPosToMap()` without matching draw-transform tests.
- Constraining `ZOOM_FACTORS` to integer-looking values without proving that command targeting and visible-map bounds remain correct.
- Using texture size helpers such as `calcPreferredTextureSize()` as presentation scaling policy. Power-of-two texture allocation is not a viewport policy.

### Files/classes that must not be touched for a first scaling/filter option

- `libs/s25main/GlobalGameSettings.*`
- `libs/s25main/network/GameMessages.*`
- `libs/s25main/network/GameClient.*` except read-only verification
- `libs/s25main/network/GameServer.*` except read-only verification
- savegame/replay serializers and deserializers
- `libs/s25main/world/GameWorldBase.*`, `GameWorld.*`, `MapLoader.*`
- `libs/s25main/gameTypes/GameSettingTypes.*`
- addon classes under `libs/s25main/addons`
- map generator settings and map serialization
- CMake, packaging, updater, release scripts
- any `data/RTTR/assets` or original-game asset files

### Main risks

- A "graphics addon" would become synchronized game configuration. That is a design error, not just an implementation detail.
- Asset override changes can accidentally alter archive entry types, indices, or load precedence.
- Integer scaling can break mouse coordinate conversion if viewport or projection changes are not mirrored in input mapping.
- Texture filtering must cover individual textures, smart bitmaps, and shared packed textures or the result will be inconsistent.
- Changing terrain renderer data structures risks replay/network drift if it touches world-derived geometry or visibility updates rather than sampler state.
- UI scaling and ingame zoom are related but separate; mixing them without tests will create hard-to-debug click/selection errors.
- Letterboxing can make UI controls visually offset from their hit boxes if mouse coordinates remain full-window coordinates.
- A render-to-texture pass can blur pixel art if the intermediate texture, final sampler, or viewport dimensions are not pinned to integer math.
- Fullscreen and windowed paths may report different window/render sizes under DPI scaling, especially when GUI scale is automatic.

## Safe first step recommendation

Texture filtering v1 is already implemented. The next safe step is not a user-facing pixel-perfect toggle. It is a small local prototype that applies the tested integer viewport math to presentation only, behind developer-only code or a non-persistent experiment.

Recommended first implementation candidate:

- Keep `helpers::CalculateIntegerScaleViewport()` as the pure calculation source.
- Prototype a presentation rectangle either in `VideoDriverWrapper::RenewViewport()` or, more safely, in a render-to-texture presentation pass.
- Prefer render-to-texture for a first real implementation if full-world and full-UI scaling must be pixel-perfect together; it creates one presentation boundary instead of making every draw path integer-aware.
- If experimenting directly in viewport setup, transform mouse/window coordinates through the same viewport rectangle before UI and world code see them.
- Keep `GameWorldView` world zoom unchanged at first. Do not combine world zoom changes with presentation scaling in the same patch.
- Keep the option local-only under `Settings::video` only after tests prove the path. Do not use addons or `GlobalGameSettings`.

Required tests before enabling UI:

- Pure helper tests for scale, centering, invalid dimensions, and too-small targets.
- Driver-level tests for screen-to-view and view-to-screen translation with a non-origin presentation viewport.
- UI hit tests for buttons, modal windows, dragging, snapping, and cursor hover in a letterboxed viewport.
- `GameWorldView::ViewPosToMap()` tests for default, min, and max zoom with presentation viewport offsets.
- Ingame integration tests for context click selection, road building, grab-and-drag scrolling, and action-window placement.
- Resize tests covering windowed, fullscreen, automatic GUI scale, and fixed GUI scale.

What must remain local-only:

- Pixel-perfect/integer scaling mode.
- Texture filtering mode.
- GUI scale and DPI presentation behavior.
- Any presentation viewport or letterbox state.
- Manual visual-test preferences and screenshots.
- No savegame, replay, network, map, addon, `.chaos` metadata, compatibility gate, or feature-key state.

Manual visual test matrix for a future implementation:

- Resolutions: 1280x720, 1920x1080, 2560x1440, 3840x2160.
- Display mode: windowed and fullscreen.
- GUI scale: normal and high.
- World zoom: min, default, and max.
- Texture filtering: `Pixel / sharp` and `Smooth`.
- Shared textures: on and off.
- VBO: on and off.
- Backends: SDL2 and WinAPI when available.
- Scenes: main menu, options graphics tab, loading screen, ingame map, dense buildings, moving workers, water/terrain edges, minimap, action windows, road-building overlay, productivity/name overlays, chat/settings windows, and pause overlay.

### Implementation follow-up: local texture filtering v1

Chaos Edition now has a local `Settings::video` texture filtering option with two modes:

- `Pixel / sharp`: keeps the conservative default and maps settings-controlled texture uploads to nearest filtering.
- `Smooth`: opt-in linear filtering for settings-controlled texture uploads.

The option is stored only in local video settings (`CONFIG.INI` video section). It is not part of `GlobalGameSettings`, addons, savegames, network sync, replay metadata, `.chaos` metadata, compatibility gates, feature keys, or asset lookup/replacement.

v1 applies centrally at OpenGL texture upload/filter assignment for regular archive bitmap textures that follow the video setting, `glSmartBitmap`, and packed shared textures from `glTexturePacker`. Existing explicit legacy smooth opt-ins, such as splash/minimap paths using `setInterpolateTexture(false)`, remain explicit smooth render behavior so the default does not regress prior presentation.

Coverage hardening adds deterministic config parsing for numeric values and readable aliases, helper-level nearest/linear tests, local settings roundtrips for both modes, invalid-value fallback to Pixel, a driver-level filter refresh test, and a UI construction test that verifies the graphics options expose `Texture filtering`, `Pixel / sharp`, and `Smooth`.

## Proposed staged roadmap

### Stage 1: Local texture filtering option

Scope:

- Add a local `Settings::video` option.
- Add one graphics-tab selector.
- Apply sampler state consistently across individual textures, smart bitmaps, and packed textures.

Risk:

- Low if it only changes OpenGL texture parameters and local config persistence.

Do not:

- Touch `GlobalGameSettings`, addons, savegames, replays, maps, network messages, or assets.

### Stage 2: Pixel-perfect / integer scaling investigation

Scope:

- Keep pure integer viewport math in `helpers::CalculateIntegerScaleViewport()`.
- Prototype presentation scaling without adding a persistent setting.
- Decide between full-window viewport constraints and render-to-texture presentation.
- Verify mouse-to-map, UI hit testing, scrolling, and resize behavior before any UI option exists.
- Keep world zoom and GUI scale as separate concepts.

Risk:

- Medium. The hard part is not drawing pixels; it is keeping coordinates correct.

Exit criteria:

- The implementation path is chosen and documented.
- Automated tests cover coordinate transforms for letterboxed viewports.
- Manual screenshots prove crisp pixel-art behavior at the defined matrix.
- The future UI option can be described as `Auto`, `Integer / pixel-perfect`, and `Free / current behavior` without ambiguity.

### Stage 3: UI HiDPI polish

Scope:

- Audit fixed UI backgrounds, splash/load screens, button bars, and image controls.
- Improve local sizing/cropping/drawing behavior before any asset replacement.
- Keep existing assets and resource ids.

Risk:

- Low to medium, depending on whether only presentation rectangles change.

### Stage 4: Optional asset override lookup with fallback

Scope:

- Only after documenting why the existing `assets/base`, `assets/overrides`, addon assets, and user `LSTS` override layers are insufficient.
- If needed, add a Chaos-only local presentation asset lookup with explicit fallback.

Risk:

- Medium to high. Archive entry types and indices are fragile.

### Stage 5: HD asset pack support

Scope:

- Separate package format, explicit compatibility rules, no original asset redistribution.
- Strict fallback to original assets.
- Versioned metadata and diagnostics.

Risk:

- High. This is not a first implementation block.

## Current pipeline overview

```text
Original/RTTR files and folders
  -> ArchiveLocator resolves base + overrides
  -> ArchiveLoader loads files/folders with libsiedler2
  -> GlAllocator creates glArchivItem_* objects
  -> Loader caches archives and typed accessors
  -> Loader::fillCaches composes glSmartBitmap caches
  -> Texture creation in glArchivItem_BitmapBase / glSmartBitmap / glTexturePacker
  -> Drawing through TerrainRenderer, GameWorldView, Window/controls, and OpenGLRenderer
  -> VideoDriverWrapper viewport/projection/buffer swap
```

## Audit conclusion

The codebase already has enough local rendering and settings structure for a small, safe HD-graphics first step. The uncomfortable part is that "HD graphics" as a broad concept is too vague and too asset-heavy. Treating it as an addon would couple presentation to synchronized gameplay state. Treating it as an asset replacement project first would create loader and compatibility risk before any user-visible scaling control exists.

Local texture filtering is the completed first step. Pixel-perfect/integer scaling should stay in investigation until the project proves the presentation rectangle and input mapping together. The first safe candidate is a local presentation-scaling prototype backed by `helpers::CalculateIntegerScaleViewport()`, followed by driver/UI/world-coordinate tests. Only then should Chaos Edition expose a persistent local video option.
