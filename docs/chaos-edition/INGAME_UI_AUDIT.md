# Ingame UI Menu and HUD Data Audit

## Executive summary

The current developer ingame UI preview is still too detached from the real ingame UI. The real structure is not a
single "modern HUD" surface. It is split across:

- the lower `dskGameInterface` button bar for minimap, main selection, construction aid, and post;
- the `iwMainMenu` "Main selection" window for economy, statistics, military, ship, diplomacy, build-order, and options
  windows;
- the context-sensitive `iwAction` activity window for build, flag, road, attack, observe, and selected-point actions;
- direct building windows opened from `dskGameInterface::ContextClick`;
- keyboard shortcuts in `dskGameInterface::Msg_KeyDown`.

The safe migration target is therefore not "replace the whole old ingame menu first". The first sane target is to
reduce the lower bar and `iwMainMenu` window friction while leaving `iwAction`, road-building, building detail windows,
simulation commands, network commands, save/replay, settings, and minimap rendering behavior untouched.

`LiveDeveloperHudDataProvider` now reads a small set of real read-only values: active player, soldier counts by rank,
armored soldiers, gold, coins, swords, food, boards, stones, unread post count, map size, selected point, and selected
object type. It does not read worker totals, total wares, latest message text, minimap toggle state, military capacity,
military readiness/status, command eligibility, or economy/storage pressure. Those preview/export fields are therefore
placeholders or explicitly not safely accessible, not proven game data.

## Current old ingame menu structure

### Lower ingame button bar

Source: `libs/s25main/desktops/dskGameInterface.cpp`

Constructor buttons:

| Button | Source | UI action | Result type |
| --- | --- | --- | --- |
| `ID_btMap` | `dskGameInterface.cpp:143` | Lower bar map button | Toggles `iwMinimap` window |
| `ID_btOptions` | `dskGameInterface.cpp:146` | Lower bar main selection button | Toggles `iwMainMenu` window |
| `ID_btConstructionAid` | `dskGameInterface.cpp:149` | Lower bar construction-aid mode | Calls `GameWorldView::ToggleShowBQ()` |
| `ID_btPost` | `dskGameInterface.cpp:153` | Lower bar post office button | Toggles `iwPostWindow`, then clears the highlighted post icon state |
| `ID_txtNumMsg` | `dskGameInterface.cpp:157` | Message count overlay | Updated by `UpdatePostIcon()` |

Click handling:

| Function | Source | What it opens or changes | Result type |
| --- | --- | --- | --- |
| `dskGameInterface::Msg_ButtonClick` | `dskGameInterface.cpp:338` | Dispatches lower-bar button clicks | Window/toggle |
| `ID_btMap` branch | `dskGameInterface.cpp:342` | `WINDOWMANAGER.ToggleWindow(std::make_unique<iwMinimap>(minimap, gwv))` | Window |
| `ID_btOptions` branch | `dskGameInterface.cpp:343` | `WINDOWMANAGER.ToggleWindow(std::make_unique<iwMainMenu>(gwv, GAMECLIENT))` | Window |
| `ID_btConstructionAid` branch | `dskGameInterface.cpp:344` | `gwv.ToggleShowBQ()` while desktop is active | Toggle |
| `ID_btPost` branch | `dskGameInterface.cpp:348` | `iwPostWindow`, `UpdatePostIcon(GetPostBox().GetNumMsgs(), false)` | Window plus UI state |

Persistent windows after screen switch are restored in `dskGameInterface::ShowPersistentWindowsAfterSwitch`
(`dskGameInterface.cpp:259`). That restoration list is a useful source of which ingame windows are established
long-lived menu surfaces: chat, post, distribution, build order, transport, military, tools, inventory, minimap,
buildings, building productivities, music player, statistics, economic progress, diplomacy, ship, and merchandise
statistics.

### Main selection window

Source: `libs/s25main/ingameWindows/iwMainMenu.cpp`

`iwMainMenu` is the old "Main selection" submenu opened by the lower bar or by the `m` hotkey. It is not a build menu.
It is a dispatcher to management/statistics/detail windows.

| Button id | Label / old function | Source | Opens | Result type |
| --- | --- | --- | --- | --- |
| `0` | Distribution of goods | `iwMainMenu.cpp:38`, `iwMainMenu.cpp:109` | `iwDistribution` | Window, command-capable settings |
| `1` | Transport | `iwMainMenu.cpp:41`, `iwMainMenu.cpp:114` | `iwTransport` | Window, command-capable settings |
| `2` | Tools | `iwMainMenu.cpp:44`, `iwMainMenu.cpp:119` | `iwTools` | Window, command-capable settings |
| `3` | General statistics | `iwMainMenu.cpp:47`, `iwMainMenu.cpp:124` | `iwStatistics` | Read-only window |
| `4` | Merchandise statistics | `iwMainMenu.cpp:50`, `iwMainMenu.cpp:129` | `iwMerchandiseStatistics` | Read-only window |
| `5` | Buildings | `iwMainMenu.cpp:52`, `iwMainMenu.cpp:134` | `iwBuildings` | Window, may jump/open building windows |
| `6` | Stock | `iwMainMenu.cpp:55`, `iwMainMenu.cpp:139` | `iwInventory` | Read-only inventory window |
| `7` | Productivity | `iwMainMenu.cpp:58`, `iwMainMenu.cpp:144` | `iwBuildingProductivities` | Read-only productivity window |
| `8` | Military | `iwMainMenu.cpp:61`, `iwMainMenu.cpp:149` | `iwMilitary` | Window, command-capable settings |
| `9` | Ship register | `iwMainMenu.cpp:64`, `iwMainMenu.cpp:154` | `iwShip` for ship id 0 | Detail window |
| `10` | Building sequence | `iwMainMenu.cpp:68`, `iwMainMenu.cpp:160` | `iwBuildOrder` if addon enabled | Window, command-capable settings |
| `11` | Diplomacy | `iwMainMenu.cpp:73`, `iwMainMenu.cpp:165` | `iwDiplomacy` | Window, command-capable diplomacy |
| `12` | Economic Progress | `iwMainMenu.cpp:77`, `iwMainMenu.cpp:170` | `iwEconomicProgress` if economy handler exists | Read-only/progress window |
| `13` | AI Debug | `iwMainMenu.cpp:84`, `iwMainMenu.cpp:175` | `iwAIDebug` for host/debug conditions | Debug window |
| `30` | Options | `iwMainMenu.cpp:97`, `iwMainMenu.cpp:192` | `iwOptionsWindow` | Options window |

### Context / activity window

Source: `libs/s25main/ingameWindows/iwAction.cpp`

`iwAction` is the real context menu for map actions. This is where most risky simulation-affecting commands live.
It should not be flattened into a permanent HUD until command eligibility and selected-point state are represented by a
real read-only contract.

Tabs:

| Tab | Source | Function group | Result type |
| --- | --- | --- | --- |
| `TAB_BUILD` | `iwAction.cpp:36`, `iwAction.cpp:83` | Build house/hut/castle/mine/harbor choices from `BuildingQuality` | Command UI |
| `TAB_SETFLAG` | `iwAction.cpp:37`, `iwAction.cpp:251` | Erect flag and optional road upgrade | Command UI |
| `TAB_WATCH` | `iwAction.cpp:38`, `iwAction.cpp:302` | Observation, house names/productivity, HQ jump, notify allies | Window/toggle/command |
| `TAB_FLAG` | `iwAction.cpp:39`, `iwAction.cpp:201` | Build road/waterway, pull down flag/building, geologist, scout | Command UI |
| `TAB_CUTROAD` | `iwAction.cpp:40`, `iwAction.cpp:269` | Dig up road and optional road upgrade | Command UI |
| `TAB_ATTACK` | `iwAction.cpp:41`, `iwAction.cpp:284` | Land attack controls | Command UI |
| `TAB_SEAATTACK` | `iwAction.cpp:42`, `iwAction.cpp:292` | Sea attack controls | Command UI |
| `TAB_CHEAT` | `iwAction.cpp:43`, `iwAction.cpp:305` | Cheat-only headquarters placement | Debug/cheat command |

Command dispatch:

| Function | Source | Real command/window |
| --- | --- | --- |
| `Msg_ButtonClick_TabBuild` | `iwAction.cpp:684` | `GAMECLIENT.SetBuildingSite(...)` |
| `Msg_ButtonClick_TabSetFlag` | `iwAction.cpp:699` | `GAMECLIENT.SetFlag(...)` or `DoUpgradeRoad()` |
| `DoUpgradeRoad` | `iwAction.cpp:355` | `GAMECLIENT.UpgradeRoad(...)` |
| `Msg_ButtonClick_TabCutRoad` | `iwAction.cpp:716` | `GAMECLIENT.DestroyRoad(...)` or `DoUpgradeRoad()` |
| `Msg_ButtonClick_TabFlag` | `iwAction.cpp:620` | `GI_StartRoadBuilding(...)`, `GAMECLIENT.DestroyFlag(...)`, `iwDemolishBuilding`, `GAMECLIENT.CallSpecialist(...)` |
| `Msg_ButtonClick_TabAttack` | `iwAction.cpp:546` | `GAMECLIENT.Attack(...)` |
| `Msg_ButtonClick_TabSeaAttack` | `iwAction.cpp:583` | `GAMECLIENT.SeaAttack(...)` |
| `Msg_ButtonClick_TabWatch` | `iwAction.cpp:738` | `iwObservate`, `gwv.ToggleShowNamesAndProductivity()`, `gwv.MoveToMapPt(HQ)`, `GAMECLIENT.NotifyAlliesOfLocation(...)` |
| `Msg_ButtonClick_TabCheat` | `iwAction.cpp:764` | `GI_GetCheats().placeCheatBuilding(...)` |

### Road construction window

Source: `libs/s25main/ingameWindows/iwRoadWindow.cpp`

`iwRoadWindow` appears only during active road-building mode. It contains "erect flag" / commit and cancel actions:

| Button id | Source | UI action | Result type |
| --- | --- | --- | --- |
| `0` | `iwRoadWindow.cpp:25`, `iwRoadWindow.cpp:39` | Build flag and road | Calls `GameInterface::GI_BuildRoad()`, which calls `GAMECLIENT.BuildRoad(...)` |
| `1` | `iwRoadWindow.cpp:20`, `iwRoadWindow.cpp:44` | Interrupt road building | Calls `GameInterface::GI_CancelRoadBuilding()` |

`dskGameInterface::GI_StartRoadBuilding` (`dskGameInterface.cpp:1017`) changes local road-building mode and cursor.
`dskGameInterface::GI_BuildRoad` (`dskGameInterface.cpp:1166`) dispatches the actual `GAMECLIENT.BuildRoad(...)`
command. This is simulation-sensitive and should stay behind the existing flow until tested separately.

### Direct selected-object windows

Source: `libs/s25main/desktops/dskGameInterface.cpp:492`

`ContextClick` first handles active road building, then selected objects:

| Selected target | Source | Opens / action | Result type |
| --- | --- | --- | --- |
| Ship at selected point | `dskGameInterface.cpp:562` | `iwShip` | Detail window |
| Own headquarters | `dskGameInterface.cpp:579` | `iwHQ` | Detail/command window |
| Own storehouse | `dskGameInterface.cpp:583` | `iwBaseWarehouse` | Detail/command window |
| Own harbor building | `dskGameInterface.cpp:587` | `iwHarborBuilding` | Detail/command window |
| Own military building | `dskGameInterface.cpp:591` | `iwMilitaryBuilding` | Detail/command window |
| Own temple | `dskGameInterface.cpp:595` | `iwTempleBuilding` | Detail/command window |
| Own usual building | `dskGameInterface.cpp:598` | `iwBuilding` | Detail window |
| Own building site | `dskGameInterface.cpp:603` | `iwBuildingSite` | Detail window |
| Allied warehouse when trade addon is enabled | `dskGameInterface.cpp:667` | `iwTrade` | Command window |
| Enemy attackable building | `dskGameInterface.cpp:679` | Adds attack/sea attack tabs to `iwAction` | Command UI |

## Shortcut and hotkey paths

Source: `libs/s25main/desktops/dskGameInterface.cpp:799`

| Input | Source | UI action | Result type |
| --- | --- | --- | --- |
| `Return` | `dskGameInterface.cpp:806` | Open `iwChat` | Window |
| Arrow keys | `dskGameInterface.cpp:810` | Scroll map view | View movement |
| `F2` | `dskGameInterface.cpp:823` | Toggle `iwSave` | Save window |
| `F3` | `dskGameInterface.cpp:826` | Toggle `iwMapDebug`, may disable FoW in replay | Debug window / replay visual state |
| `F8` | `dskGameInterface.cpp:834` | Toggle keyboard layout text file | Help/document window |
| `F9` | `dskGameInterface.cpp:837` | Toggle readme text file | Help/document window |
| `F10` | `dskGameInterface.cpp:840` | Toggle `iwSettings` | Settings window |
| `F11` | `dskGameInterface.cpp:841` | Toggle `iwMusicPlayer` | Window |
| `F12` | `dskGameInterface.cpp:844` | Toggle `iwOptionsWindow` | Options window |
| `+` / `-` | `dskGameInterface.cpp:851` | Increase/decrease speed in replay or single player | Game client action |
| `1`..`8` | `dskGameInterface.cpp:860` | Replay player switch or single-player dummy AI swap | Game client action |
| `b` | `dskGameInterface.cpp:890` | Move to last map position | View movement |
| `v` | `dskGameInterface.cpp:891` | Increase single-player speed quickly | Game client action |
| `c` | `dskGameInterface.cpp:895` | Toggle building names | Toggle |
| `d` | `dskGameInterface.cpp:898` | Toggle replay fog-of-war visibility | Replay visual state |
| `h` | `dskGameInterface.cpp:901` | Move to headquarters | View movement |
| `i` | `dskGameInterface.cpp:909` | Toggle `iwInventory` | Window |
| `j` | `dskGameInterface.cpp:912` | Toggle `iwSkipGFs` | Replay/single-player control window |
| `l` | `dskGameInterface.cpp:916` | Toggle `iwMinimap` | Window |
| `m` | `dskGameInterface.cpp:919` | Toggle `iwMainMenu` | Window |
| `n` | `dskGameInterface.cpp:922` | Toggle `iwPostWindow`, clear post icon highlight | Window plus UI state |
| `p` | `dskGameInterface.cpp:926` | Toggle pause | Game client action |
| `Alt+q` | `dskGameInterface.cpp:929` | Toggle `iwEndgame` | Dialog |
| `s` | `dskGameInterface.cpp:933` | Toggle productivity overlay | Toggle |
| `Space` | `dskGameInterface.cpp:936` | Toggle construction aid | Toggle |
| `Ctrl+z`, `z`, `Shift+z` | `dskGameInterface.cpp:940` | Reset/step zoom | View zoom |

## Menu groups and functions

| Group | Current old menu source | Current functions | Classes/functions involved | Suggested future placement | Risk |
| --- | --- | --- | --- | --- | --- |
| Build | `iwAction` build tab | Build hut/house/castle/mine/harbor building icons based on `BuildingQuality` and enabled buildings | `dskGameInterface::ContextClick`, `dskGameInterface::ShowActionWindow`, `iwAction::Msg_ButtonClick_TabBuild`, `GAMECLIENT.SetBuildingSite` | Toggle panel only when selected point can build; not permanent chip | High: sends real build commands |
| Roads/Flags | `iwAction` flag/setflag/cutroad tabs and `iwRoadWindow` | Start land/water road mode, set flag, pull down flag/building, cut road, upgrade road, commit/cancel road building | `GI_StartRoadBuilding`, `GI_BuildRoad`, `GI_CancelRoadBuilding`, `GAMECLIENT.SetFlag`, `DestroyRoad`, `UpgradeRoad`, `DestroyFlag` | Selection-driven toggle panel; road mode still needs existing modal flow | High: commands and local road-building state |
| Military | `iwMainMenu` military window and `iwAction` attack tabs | Military settings sliders; land/sea attack count and strong/weak selector | `iwMilitary`, `GAMECLIENT.ChangeMilitary`, `iwAction::Msg_ButtonClick_TabAttack`, `GAMECLIENT.Attack`, `GAMECLIENT.SeaAttack` | Military settings as toggle panel; attacks remain context detail panel | High for attacks/settings |
| Economy/Resources | `iwMainMenu` distribution, transport, tools, inventory, build order, economic progress | Distribution sliders, transport order, tool priorities/orders, inventory, build order, economic progress | `iwDistribution`, `iwTransport`, `iwTools`, `iwInventory`, `iwBuildOrder`, `iwEconomicProgress` | Resource chips in top/bottom bar; settings remain toggle panels or old windows | Medium/high: settings windows send commands |
| Messages | Lower bar post button and `n` hotkey | Post count icon, post window, category filters, goto/delete/accept/deny in post window | `dskGameInterface::GetPostBox`, `UpdatePostIcon`, `iwPostWindow`, `PostBox`, `PostMsg` | Permanent unread chip plus toggle panel; keep full post window for message actions | Medium: delete/goto/diplomacy actions in post window |
| Map/Minimap | Lower bar map button, `l` hotkey, `iwMinimap` | Minimap, territory/houses/roads toggles, expand/collapse, click-to-move viewport | `iwMinimap`, `ctrlIngameMinimap`, `IngameMinimap`, `GameWorldView::MoveToMapPt` | Toggle panel/window; do not inline renderer first | Medium: view movement and private minimap state |
| Statistics | `iwMainMenu` statistics, merchandise statistics, buildings, productivity | General statistic graphs, merchandise statistic graph, buildings list, productivities | `iwStatistics`, `iwMerchandiseStatistics`, `iwBuildings`, `iwBuildingProductivities`, `GamePlayer::GetStatistic` | Rare detail windows; selected compact summaries may become chips later | Low/medium: mostly read-only, some building-list navigation |
| Options/Game Menu | `iwMainMenu` options, hotkeys F10/F12/F2/F11/Alt+q | Ingame options, settings, save, music player, endgame dialog | `iwOptionsWindow`, `iwSettings`, `iwSave`, `iwMusicPlayer`, `iwEndgame` | Leave as old detail dialogs / top menu button | High: settings/save/endgame should not be mocked |
| Selection/Context | `dskGameInterface::ContextClick` and `iwAction` | Object/building window selection, selected-point action eligibility, observe, HQ jump | `GameWorldView::GetSelectedPt`, `GameWorldViewer`, `GameWorldBase::GetNO`, many `iw*` windows | Context panel only when selected point/object is valid | High: mixes read-only state with command dispatch |

## Current windows/dialogs opened by menu actions

Windows opened from the lower bar or `iwMainMenu`:

- `iwMinimap`
- `iwMainMenu`
- `iwPostWindow`
- `iwDistribution`
- `iwTransport`
- `iwTools`
- `iwStatistics`
- `iwMerchandiseStatistics`
- `iwBuildings`
- `iwInventory`
- `iwBuildingProductivities`
- `iwMilitary`
- `iwShip`
- `iwBuildOrder`
- `iwDiplomacy`
- `iwEconomicProgress`
- `iwAIDebug`
- `iwOptionsWindow`

Windows opened from selected-object/context paths:

- `iwShip`
- `iwHQ`
- `iwBaseWarehouse`
- `iwHarborBuilding`
- `iwMilitaryBuilding`
- `iwTempleBuilding`
- `iwBuilding`
- `iwBuildingSite`
- `iwTrade`
- `iwAction`
- `iwRoadWindow`
- `iwDemolishBuilding`
- `iwObservate`

Windows opened from hotkeys:

- `iwChat`
- `iwSave`
- `iwMapDebug`
- `iwTextfile` for keyboard layout and readme
- `iwSettings`
- `iwMusicPlayer`
- `iwOptionsWindow`
- `iwInventory`
- `iwSkipGFs`
- `iwMinimap`
- `iwMainMenu`
- `iwPostWindow`
- `iwEndgame`

## Real read-only data sources

The following sources are real and already usable or close to usable without invoking commands.

| Value | Current source | Status | Notes |
| --- | --- | --- | --- |
| Player name | `GameWorldViewer::GetPlayer()`, `GamePlayerInfo::name`, used in `LiveDeveloperHudDataProvider` | Live read-only | `DeveloperHudDataProvider.cpp:223-231` |
| Active player id | `GameWorldViewer::GetPlayerId()` | Live read-only | `DeveloperHudDataProvider.cpp:231` |
| Soldier total | Sum of `Inventory` entries for `SOLDIER_JOBS` | Live read-only | `DeveloperHudDataProvider.cpp:232-237`, total computed in `MakeDeveloperHudViewModel` |
| Soldier ranks | `Inventory[Job soldierJob]` for `SOLDIER_JOBS[rank]` | Live read-only | `Inventory::operator[](Job)` in `Inventory.h` |
| Armored soldiers | `Inventory[jobEnumToAmoredSoldierEnum(soldierJob)]` | Live read-only | Current provider sums all ranks |
| Gold | `player.GetInventory()[GoodType::Gold]` | Live read-only | `DeveloperHudDataProvider.cpp:238` |
| Coins | `player.GetInventory()[GoodType::Coins]` | Live read-only | `DeveloperHudDataProvider.cpp:239` |
| Swords | `player.GetInventory()[GoodType::Sword]` | Live read-only | `DeveloperHudDataProvider.cpp:240` |
| Food | `Fish + Bread + Meat` from `Inventory` | Live read-only | `DeveloperHudDataProvider.cpp:241`; this is a chosen aggregate, not a native field |
| Boards | `player.GetInventory()[GoodType::Boards]` | Live read-only | Same `Inventory` source as gold/coins/swords |
| Stones | `player.GetInventory()[GoodType::Stones]` | Live read-only | Same `Inventory` source as gold/coins/swords |
| Worker count / population | Sum over `Inventory::people` or selected `Job` entries | Safely available, not yet exposed | Needs an agreed definition of "workers" vs all people vs soldiers |
| Wares total | Sum over `Inventory::goods` | Safely available, not yet exposed | Needs filtering for unused/addon goods before becoming HUD contract |
| Unread messages | `PostBox::GetNumMsgs()` | Live read-only | Used by `dskGameInterface::UpdatePostIcon` and provider |
| Latest message | `PostBox::GetMsg(idx)->GetText()` | Safely available, not yet exposed | Requires deciding newest index and filtering mission-goal/special messages |
| Selected point | `GameWorldView::GetSelectedPt()` | Live read-only | `DeveloperHudDataProvider.cpp:244` |
| Selected object label | `GameWorldBase::GetNO(selectedPoint)->GetGOT()` mapped locally | Live read-only but coarse | `DeveloperHudDataProvider.cpp:245-246`; current label loses building subtype and ownership |
| Map size | `GameWorldViewer::GetWorld().GetSize()` | Live read-only | `DeveloperHudDataProvider.cpp:243` |
| Current viewport | `GameWorldView::GetFirstPt()`, `GetLastPt()`, `GetOffset()` | Safely available, not yet exposed | Used by minimap viewport drawing |
| Minimap rendering data | `IngameMinimap`, `Minimap`, `ctrlIngameMinimap` | Exists, not safely exported | Renderer/control object is not a data-contract source yet |
| Minimap territory/houses/roads visibility | `IngameMinimap` private booleans | Not safely accessible yet | Needs small read-only accessors if HUD must reflect toggle state |
| Economy mode progress | `GameWorldBase::getEconHandler()` and `iwEconomicProgress` | Exists, not audited deeply | Safe HUD contract needs a dedicated summary accessor |
| Economy/storage pressure | Warehouse/building/inventory internals | Not safely accessible yet | Do not infer from random inventory values |

## Values not safely accessible yet

- `military.status.label`: current live provider leaves it empty; no proven read-only summary exists.
- `military.capacity.used` and `military.capacity.max`: current provider marks these as not safely accessible when max is
  zero; do not invent capacity semantics.
- `messages.latest.label`: `PostBox` can provide messages, but the provider does not expose the latest message yet.
  A tiny read-only helper can be added later if the desired newest-message behavior is defined.
- `selection.context.actions`: the real eligibility logic is embedded in `dskGameInterface::ContextClick` and
  `ShowActionWindow`; exporting this safely requires a read-only action-eligibility model, not duplicated command logic.
- `minimap.thumbnail`: the renderer exists, but there is no current export for a HUD thumbnail.
- `minimap.viewport`: first/last drawn points and offset exist, but the provider does not expose them.
- `minimap toggle state`: territory/houses/roads booleans are private inside `IngameMinimap`.
- `economy.storage.pressure` and `economy.production.alerts`: no small stable read-only accessor was found in this audit.
- `worker count` and `wares total`: raw arrays are safely readable, but the HUD contract needs a definition before showing
  totals. Otherwise it will become another misleading mock value.

## Proposed HUD data contract corrections

### Realistic existing fields

These fields are realistic because `LiveDeveloperHudDataProvider` already has real read-only sources:

- `player.active.id`
- `player.active.name`
- `player.active.label`
- `military.soldiers.total`
- `military.soldiers.ranks`
- `military.soldiers.armored`
- `resources.gold`
- `resources.coins`
- `resources.swords`
- `resources.food`
- `resources.boards`
- `resources.stones`
- `messages.unread.count`
- `messages.lane.text`
- `selection.map.point`
- `selection.object.summary` as a coarse object-type label
- `map.size`

### Realistic but still missing from the current provider

These should be added before any new mock UI claims they exist:

- `population.total` or `workers.total`, after defining whether soldiers and inactive/unused addon jobs count.
- `wares.total`, after defining addon filtering and whether shields are nation-converted.
- `messages.latest.text`, using `PostBox::GetMsg()` only after defining newest-message index and truncation rules.
- `selection.object.type`, `selection.object.owner`, and possibly `selection.building.type` for a less vague selected
  object summary.
- `map.viewport.center` or `map.viewport.bounds`, based on `GameWorldView::GetFirstPt()` and `GetLastPt()`.

### Mock-heavy or misleading fields

These fields should not drive a new UI until backed by real sources:

- `military.status.label`: no mock readiness label is exported anymore; it stays not safely accessible.
- `military.capacity.used` and `military.capacity.max`: no numeric capacity is exported anymore; both stay not safely
  accessible.
- `messages.latest.label`: no plausible-looking text is exported anymore; it stays not safely accessible.
- `minimap.thumbnail`: "placeholder card" is a UI placeholder, not the real minimap.
- `minimap.viewport`: currently "not exported yet".
- `commands.rail.categories`: a design idea, not a data source.
- `commands.quick.build.labels`: static UI labels only.
- `commands.quick.build.eligibility`: not safely accessible yet. Real build actions depend on selected point,
  `BuildingQuality`, addon state, building enabled state, nearby military buildings, road mode, replay mode, and
  ownership.
- `commands.dispatch`: fine as a safety placeholder, but it must stay disabled until a real command eligibility contract
  exists.
- `economy.storage.pressure` and `economy.production.alerts`: not found as safe read-only fields.

### Fields to remove or downgrade in the next implementation block

- Remove fixed mock values from production-facing preview paths where live data exists.
- Downgrade command-rail fields to explicit "design placeholder" labels, or remove them from top-level export until the
  real `iwAction` eligibility model is audited into a read-only structure.
- Replace "military capacity" with the real soldier rank/armor values for now.
- Replace "latest message" mock text with either a true `PostBox` latest-message accessor or "not safely accessible yet".

## Proposed toggle-panel migration map

### Top bar candidates

Good candidates for permanent, read-only top-bar chips:

- active player label;
- total soldiers and optionally a compact rank summary;
- gold, swords, food, coins, boards, stones;
- unread post count;
- map size only in developer/debug preview, not product HUD.

These are defensible because they read from `GameWorldViewer`, `GamePlayer::GetInventory`, `PostBox`, and map size
without sending commands.

### Bottom bar candidates

Good candidates to reduce the old lower bar:

- Map/minimap toggle, preserving `iwMinimap` behavior first.
- Main selection / management toggle, initially opening existing panels or old windows.
- Construction aid toggle (`ToggleShowBQ`).
- Post toggle with unread count.

This is the closest real replacement target for the old lower menu because it maps directly to
`dskGameInterface::Msg_ButtonClick`.

### Toggle panels

Good candidates for toggle panels after the data contract is corrected:

- Economy/settings panel that opens or embeds distribution, transport, tools, build order, and inventory links. Do not
  rewrite the command dispatch yet.
- Military settings panel for the `iwMilitary` sliders, with attacks kept context-only.
- Messages panel showing unread count/latest message, with the full `iwPostWindow` preserved for delete/goto/diplomacy
  actions until those are separately audited.
- Selection context panel that lists real eligible actions from a future read-only action model.
- Build/Roads panel only when selected point state proves the real actions are available.

### Keep as detail windows for now

Keep these as old windows/dialogs in the next implementation block:

- save, settings, options, music player, endgame;
- building, warehouse, HQ, military building, harbor, temple, ship, trade, diplomacy;
- statistics graphs and merchandise statistics;
- road construction commit/cancel flow;
- attack windows;
- AI debug and map debug.

### Visible only on selection

These should be selection-dependent, not permanent:

- buildable building categories;
- set flag / build road / waterway / cut road / upgrade road;
- attack and sea attack controls;
- observe selected point;
- selected building detail shortcuts;
- notify allies of selected location.

## Risks

- The biggest risk is treating `iwAction` as a simple visual submenu. It is not. It sends real network/game commands.
- Duplicating `dskGameInterface::ContextClick` logic inside a HUD provider would create drift and command bugs. A future
  read-only action eligibility model should be extracted before any new command panel is wired.
- The current preview makes command categories look universally available. In the real game they are conditional on map
  point, ownership, visibility, building quality, addons, replay mode, water/land, nearby military buildings, and current
  road-building state.
- Minimap state is not currently a clean data source. Rendering and toggles live in `IngameMinimap`/`ctrlIngameMinimap`,
  not in the HUD provider.
- Inventory totals are easy to read but easy to misrepresent. "Food", "workers", and "wares total" must be explicitly
  defined before becoming permanent HUD values.
- Options/settings/save/replay/network paths should remain outside the first HUD migration. Touching them would expand the
  blast radius for no UI-learning benefit.

## Recommended next implementation block

1. Keep the preview visually stable. Do not add new mock values.
2. Keep `LiveDeveloperHudDataProvider` limited to clearly safe read-only values. Boards and stones are now included.
   Add worker/wares totals only after naming the exact definitions, and latest post text only after defining newest
   message index and truncation behavior.
3. Keep fake fields removed or relabeled:
   - military status/capacity;
   - quick build command lists;
   - minimap thumbnail;
   - economy/storage pressure.
4. Add a small documented `DeveloperHudSnapshot` contract section near the provider or in tests that states whether each
   field is live, placeholder, or not safely accessible.
5. For UI migration, target the lower bar and `iwMainMenu` first:
   - bottom bar: map, main selection, construction aid, post;
   - management panels: economy, military settings, messages, statistics links;
   - leave `iwAction` command tabs untouched.
6. Create a separate future audit for a read-only selected-action eligibility model before moving build/road/attack
   commands into any new HUD panel.
