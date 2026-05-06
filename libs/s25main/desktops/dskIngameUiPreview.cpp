// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dskIngameUiPreview.h"
#include "Loader.h"
#include "Settings.h"
#include "TextureFiltering.h"
#include "WindowManager.h"
#include "controls/ctrlButton.h"
#include "controls/ctrlText.h"
#include "controls/ctrlTextButton.h"
#include "desktops/dskTest.h"
#include "driver/KeyEvent.h"
#include "drivers/VideoDriverWrapper.h"
#include "ogl/FontStyle.h"
#include "ogl/glFont.h"
#include "s25util/colors.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <utility>

namespace {
enum
{
    ID_txtTitle = 1,
    ID_txtNotice,
    ID_txtSize,
    ID_txtGuiScale,
    ID_txtFiltering,
    ID_txtActiveState,
    ID_txtStateHint,
    ID_txtHudDataSource,
    ID_txtHudCompactData,
    ID_txtHudResourceData,
    ID_txtHudMessageData,
    ID_txtHudContractData,
    ID_txtHudAvailabilityData,
    ID_txtStateLine0,
    ID_txtStateLine1,
    ID_txtStateLine2,
    ID_txtStateLine3,
    ID_txtStateLine4,
    ID_txtStateLine5,
    ID_txtStateLine6,
    ID_txtStateLine7,
    ID_btStateClassicCompactHud,
    ID_btStateToggleBuildRoads,
    ID_btStateToggleMilitaryEconomy,
    ID_btStateMessagesMinimap,
    ID_btStateSelectionContext,
    ID_btStateSmallScreenStress,
    ID_btToggleBuild,
    ID_btToggleRoads,
    ID_btToggleMilitary,
    ID_btToggleEconomy,
    ID_btToggleMessages,
    ID_btToggleMap,
    ID_btToggleSelection,
    ID_btActionPrimary,
    ID_btActionSecondary,
    ID_btActionDisabled,
    ID_btBack
};

constexpr int WORKBENCH_HEADER_HEIGHT = 118;
constexpr int TOP_HUD_HEIGHT = 38;
constexpr int BOTTOM_HUD_HEIGHT = 58;
constexpr unsigned COLOR_EARTH_DARK = 0xE01E2116;
constexpr unsigned COLOR_EARTH_PANEL = 0xE03A3021;
constexpr unsigned COLOR_EARTH_PANEL_ALT = 0xD0453A27;
constexpr unsigned COLOR_EARTH_EDGE = 0xFFE3C681;
constexpr unsigned COLOR_EARTH_EDGE_DIM = 0xB89D8154;
constexpr unsigned COLOR_CHIP = 0xDD5A4B2E;
constexpr unsigned COLOR_CHIP_ACTIVE = 0xEE75643C;
constexpr unsigned COLOR_DISABLED = 0xAA3C3A34;
constexpr unsigned COLOR_MAP_A = 0xFF355735;
constexpr unsigned COLOR_MAP_B = 0xFF436A3E;
constexpr unsigned COLOR_MAP_GRID = 0x604F7748;
constexpr unsigned COLOR_MINIMAP_WATER = 0xFF244966;
constexpr unsigned COLOR_MINIMAP_LAND = 0xFF3E6833;
constexpr unsigned COLOR_MINIMAP_FIELD = 0xFF987A38;
constexpr unsigned COLOR_WARNING = 0xFFD67B4A;
constexpr unsigned COLOR_DANGER = 0xFFD85038;

struct PreviewStateDefinition
{
    dskIngameUiPreview::PreviewState state;
    unsigned buttonId;
    const char* buttonLabel;
    char32_t shortcut;
    const char* label;
    std::array<const char*, 8> lines;
};

const std::array<PreviewStateDefinition, static_cast<unsigned>(dskIngameUiPreview::PreviewState::Count)>
  PREVIEW_STATES = {
    {{dskIngameUiPreview::PreviewState::ClassicCompactHud,
      ID_btStateClassicCompactHud,
      "1 Compact HUD",
      U'1',
      "Classic-inspired Compact HUD",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Modernized classic toggle-panel UI: narrow top bar, narrow bottom bar, small permanent values",
        "Toggle buttons: Build, Roads, Military, Economy, Messages, Map, Selection",
        "Resource chips use resources group; soldier chip uses military group; message chip uses messages group",
        "Warm classic-inspired panels, moderate frame weight, compact pixel-friendly labels",
        "No legacy main/submenu wall and no sci-fi command center treatment",
        "Availability remains subtle: mock / live read-only / placeholder / not safely accessible yet",
        "No real map, simulation, network, save, replay, settings, addons, or commands are touched"}}},
     {dskIngameUiPreview::PreviewState::ToggleBuildRoads,
      ID_btStateToggleBuildRoads,
      "2 Build/Roads",
      U'2',
      "Toggle Panels: Build/Roads",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Build and Roads buttons act as on/off toggles for compact classic-style panels",
        "House category and road/flag tools are represented with compact action buttons",
        "Commands group is visible but dispatch stays disabled / no commands",
        "Includes active, available, and disabled visual states without starting placement",
        "Later UI question: can direct toggles replace deep Main Menu -> Build Menu friction?",
        "No terrain, ownership, pathfinding, addons, settings, or game command dispatch",
        "Panel language is short labels and glyph-like initials, not a text-heavy mock page"}}},
     {dskIngameUiPreview::PreviewState::ToggleMilitaryEconomy,
      ID_btStateToggleMilitaryEconomy,
      "3 Mil/Eco",
      U'3',
      "Toggle Panels: Military/Economy",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Military panel uses military group: soldiers, ranks, readiness, capacity",
        "Economy panel uses resources and economy groups: gold, swords, food, storage pressure",
        "Important values are readable but intentionally small enough to keep the map dominant",
        "Mock fields are visible; unavailable economy summaries stay marked not safely accessible yet",
        "No barracks, production, combat, inventory mutation, or simulation logic is queried",
        "Later UI question: which values deserve permanent chips versus on-demand panels?",
        "Classic strategy-game mood with cleaner grouping and lighter frame weight"}}},
     {dskIngameUiPreview::PreviewState::MessagesMinimap,
      ID_btStateMessagesMinimap,
      "4 Msg/Map",
      U'4',
      "Messages + Minimap",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Messages panel uses messages group: unread count, latest label, compact mute control",
        "Small toast preview is visible without becoming a blocking modal window",
        "Map panel uses map group: map size plus static minimap placeholder card",
        "Minimap placement previews bottom-left and top-left conflict zones in one reduced state",
        "Zoom + / Zoom - / collapse / expand are static preview controls only",
        "No real map data, fog, viewport, GameWorldView minimap renderer, or CONFIG.INI changes",
        "Later UI question: should minimap be docked, toggled, or collapsible by default?"}}},
     {dskIngameUiPreview::PreviewState::SelectionContext,
      ID_btStateSelectionContext,
      "5 Selection",
      U'5',
      "Selected Object / Context Panel",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Selection button opens a compact context panel instead of a permanent large menu",
        "Selection group feeds selected object summary and selected map point",
        "2-5 action buttons are represented; command dispatch remains disabled",
        "Inventory snippets reuse resource chips where relevant: gold, swords, food, coins",
        "The panel is contextual, small, and easy to dismiss through the same toggle button",
        "No object command, demolition, upgrade, production toggle, or game state mutation",
        "Later UI question: which context actions are worth showing immediately?"}}},
     {dskIngameUiPreview::PreviewState::SmallScreenStress,
      ID_btStateSmallScreenStress,
      "6 Small",
      U'6',
      "Small-screen Stress",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Reduced-state stress view shows compact bars, toggles, messages, minimap, and context panel together",
        "Pressure target: 1280x720 style safe-area thinking without changing real resolution",
        "Checks whether bars stay small and values stay readable without dominating the map",
        "Highlights conflicts between top-left minimap, toast, and edge/context panels",
        "No screenshot-comparison, no real UI scaling write, no CONFIG.INI or integer-scaling change",
        "Later UI question: is the direction too modern, too old, or exactly between?",
        "If this feels cramped here, fix it before product UI work"}}}}};

const PreviewStateDefinition& GetPreviewStateDefinition(const dskIngameUiPreview::PreviewState state)
{
    const auto stateIt =
      std::find_if(PREVIEW_STATES.begin(), PREVIEW_STATES.end(),
                   [state](const PreviewStateDefinition& definition) { return definition.state == state; });
    return stateIt != PREVIEW_STATES.end() ? *stateIt : PREVIEW_STATES.front();
}

std::size_t GetPreviewStateIndex(const dskIngameUiPreview::PreviewState state)
{
    return static_cast<std::size_t>(&GetPreviewStateDefinition(state) - PREVIEW_STATES.data());
}

const DeveloperHudDataGroup* FindGroup(const DeveloperHudViewModel& data, const std::string& key)
{
    const auto groupIt = std::find_if(data.developmentExportGroups.begin(), data.developmentExportGroups.end(),
                                      [&key](const DeveloperHudDataGroup& group) { return group.key == key; });
    return groupIt != data.developmentExportGroups.end() ? &*groupIt : nullptr;
}

const DeveloperHudDataField* FindField(const DeveloperHudViewModel& data, const std::string& groupKey,
                                       const std::string& fieldKey)
{
    const DeveloperHudDataGroup* group = FindGroup(data, groupKey);
    if(!group)
        return nullptr;

    const auto fieldIt =
      std::find_if(group->fields.begin(), group->fields.end(),
                   [&fieldKey](const DeveloperHudDataField& field) { return field.key == fieldKey; });
    return fieldIt != group->fields.end() ? &*fieldIt : nullptr;
}

std::string GetAvailabilityLabel(const DeveloperHudViewModel& data, const std::string& groupKey,
                                 const std::string& fieldKey)
{
    const DeveloperHudDataField* field = FindField(data, groupKey, fieldKey);
    return field ? ToString(field->availability) : "missing";
}

std::string GetTextureFilteringLabel()
{
    switch(SETTINGS.video.textureFiltering)
    {
        case TextureFiltering::Pixel: return "Texture filtering: Pixel / sharp";
        case TextureFiltering::Smooth: return "Texture filtering: Smooth";
    }
    return "Texture filtering: unknown";
}

std::string GetGuiScaleLabel()
{
    return "GUI scale: " + std::to_string(VIDEODRIVER.getGuiScale().percent()) + "%";
}

std::string GetPreviewSizeLabel()
{
    const Extent renderSize = VIDEODRIVER.GetRenderSize();
    return "Preview size: " + std::to_string(renderSize.x) + "x" + std::to_string(renderSize.y);
}

} // namespace

void dskIngameUiPreview::DrawClassicPanel(const DrawPoint pos, const Extent size, const std::string& title) const
{
    Draw3D(Rect(pos, size), TextureColor::Green1, true);
    DrawRectangle(Rect(pos + DrawPoint(6, 6), Extent(size.x - 12, 22)), COLOR_EARTH_PANEL_ALT);
    SmallFont->Draw(pos + DrawPoint(14, 11), title, FontStyle{}, COLOR_EARTH_EDGE);
}

void dskIngameUiPreview::DrawChip(const DrawPoint pos, const Extent size, const std::string& label,
                                  const bool active) const
{
    Draw3D(Rect(pos, size), active ? TextureColor::Green2 : TextureColor::Grey, false);
    DrawRectangle(Rect(pos + DrawPoint(3, 3), Extent(size.x - 6, size.y - 6)), active ? COLOR_CHIP_ACTIVE : COLOR_CHIP);
    SmallFont->Draw(pos + DrawPoint(size.x / 2, 6), label, FontStyle::CENTER, COLOR_YELLOW);
}

void dskIngameUiPreview::DrawMiniMap(const DrawPoint pos, const Extent size) const
{
    DrawRectangle(Rect(pos, size), COLOR_MINIMAP_WATER);
    DrawRectangle(Rect(pos + DrawPoint(size.x / 10, size.y / 9), Extent(size.x * 3 / 5, size.y / 2)),
                  COLOR_MINIMAP_LAND);
    DrawRectangle(Rect(pos + DrawPoint(size.x / 2, size.y / 2), Extent(size.x / 3, size.y / 4)), COLOR_MINIMAP_FIELD);
    DrawLine(pos, pos + DrawPoint(size.x, size.y), 1, COLOR_EARTH_EDGE_DIM);
    DrawLine(pos + DrawPoint(size.x, 0), pos + DrawPoint(0, size.y), 1, COLOR_EARTH_EDGE_DIM);
}

void dskIngameUiPreview::DrawActionGrid(const DrawPoint pos, const std::array<const char*, 8>& labels,
                                        const unsigned activeIndex, const unsigned disabledIndex) const
{
    for(unsigned i = 0; i < labels.size(); ++i)
    {
        const DrawPoint buttonPos = pos + DrawPoint(static_cast<int>(i % 4) * 70, static_cast<int>(i / 4) * 32);
        const bool active = i == activeIndex;
        const bool disabled = i == disabledIndex;
        Draw3D(Rect(buttonPos, Extent(62, 24)),
               disabled ? TextureColor::Grey : (active ? TextureColor::Green2 : TextureColor::Green1), false);
        DrawRectangle(Rect(buttonPos + DrawPoint(3, 3), Extent(56, 18)),
                      disabled ? COLOR_DISABLED : (active ? COLOR_CHIP_ACTIVE : COLOR_CHIP));
        SmallFont->Draw(buttonPos + DrawPoint(31, 6), labels[i], FontStyle::CENTER,
                        disabled ? COLOR_GREY : COLOR_YELLOW);
    }
}

dskIngameUiPreview::dskIngameUiPreview()
    : Desktop(nullptr), previewState_(PreviewState::ClassicCompactHud),
      hudDataProvider_(std::make_unique<MockDeveloperHudDataProvider>()), hudData_(hudDataProvider_->GetViewModel())
{
    SetScale(false);
    SetFpsDisplay(false);

    AddText(ID_txtTitle, DrawPoint(0, 0), "Developer Preview: Ingame UI Workbench", COLOR_ORANGE, FontStyle::CENTER,
            LargeFont);
    AddText(ID_txtNotice, DrawPoint(0, 0),
            "Developer Preview | Static mock data | Not product UI | No gameplay logic; no map/simulation/network/save",
            COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    AddText(ID_txtSize, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtGuiScale, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtFiltering, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtActiveState, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle::CENTER, SmallFont);
    AddText(ID_txtStateHint, DrawPoint(0, 0), "Keys 1-6 select reduced review states; Tab cycles", COLOR_GREY,
            FontStyle::CENTER, SmallFont);
    AddText(ID_txtHudDataSource, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtHudCompactData, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtHudResourceData, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtHudMessageData, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtHudContractData, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtHudAvailabilityData, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);

    for(unsigned i = 0; i < 8; ++i)
        AddText(ID_txtStateLine0 + i, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);

    for(const PreviewStateDefinition& definition : PREVIEW_STATES)
    {
        AddTextButton(definition.buttonId, DrawPoint(0, 0), Extent(126, 20), TextureColor::Grey, definition.buttonLabel,
                      SmallFont);
    }

    AddTextButton(ID_btToggleBuild, DrawPoint(0, 0), Extent(58, 24), TextureColor::Grey, "Build", SmallFont);
    AddTextButton(ID_btToggleRoads, DrawPoint(0, 0), Extent(58, 24), TextureColor::Grey, "Roads", SmallFont);
    AddTextButton(ID_btToggleMilitary, DrawPoint(0, 0), Extent(70, 24), TextureColor::Grey, "Military", SmallFont);
    AddTextButton(ID_btToggleEconomy, DrawPoint(0, 0), Extent(70, 24), TextureColor::Grey, "Economy", SmallFont);
    AddTextButton(ID_btToggleMessages, DrawPoint(0, 0), Extent(72, 24), TextureColor::Grey, "Messages", SmallFont);
    AddTextButton(ID_btToggleMap, DrawPoint(0, 0), Extent(50, 24), TextureColor::Grey, "Map", SmallFont);
    AddTextButton(ID_btToggleSelection, DrawPoint(0, 0), Extent(76, 24), TextureColor::Grey, "Selection", SmallFont);

    AddTextButton(ID_btActionPrimary, DrawPoint(0, 0), Extent(86, 22), TextureColor::Green2, "Road", SmallFont);
    AddTextButton(ID_btActionSecondary, DrawPoint(0, 0), Extent(86, 22), TextureColor::Green1, "Flag", SmallFont);
    AddTextButton(ID_btActionDisabled, DrawPoint(0, 0), Extent(86, 22), TextureColor::Grey, "Disabled", SmallFont);
    AddTextButton(ID_btBack, DrawPoint(0, 0), Extent(120, 22), TextureColor::Red1, "Back", NormalFont);

    LayoutControls();
    UpdatePreviewStateControls();
}

void dskIngameUiPreview::Resize(const Extent& newSize)
{
    Window::Resize(newSize);
    LayoutControls();
}

void dskIngameUiPreview::Msg_ButtonClick(const unsigned ctrl_id)
{
    for(const PreviewStateDefinition& definition : PREVIEW_STATES)
    {
        if(ctrl_id == definition.buttonId)
        {
            SetPreviewState(definition.state);
            return;
        }
    }

    if(ctrl_id == ID_btToggleBuild || ctrl_id == ID_btToggleRoads)
        SetPreviewState(PreviewState::ToggleBuildRoads);
    else if(ctrl_id == ID_btToggleMilitary || ctrl_id == ID_btToggleEconomy)
        SetPreviewState(PreviewState::ToggleMilitaryEconomy);
    else if(ctrl_id == ID_btToggleMessages || ctrl_id == ID_btToggleMap)
        SetPreviewState(PreviewState::MessagesMinimap);
    else if(ctrl_id == ID_btToggleSelection)
        SetPreviewState(PreviewState::SelectionContext);
    else if(ctrl_id == ID_btBack)
        WINDOWMANAGER.Switch(std::make_unique<dskTest>());
}

void dskIngameUiPreview::Msg_PaintBefore()
{
    Desktop::Msg_PaintBefore();
    RefreshHudData();
    UpdateDiagnostics();
    DrawPreviewBackground();
    DrawIngameShell();
    DrawPreviewState();
}

bool dskIngameUiPreview::Msg_KeyDown(const KeyEvent& ke)
{
    switch(ke.kt)
    {
        case KeyType::Escape: WINDOWMANAGER.Switch(std::make_unique<dskTest>()); return true;
        case KeyType::Tab: CyclePreviewState(ke.shift ? -1 : 1); return true;
        case KeyType::Char:
        {
            const char32_t key = ke.c >= U'A' && ke.c <= U'Z' ? ke.c - U'A' + U'a' : ke.c;
            const auto stateIt =
              std::find_if(PREVIEW_STATES.begin(), PREVIEW_STATES.end(),
                           [key](const PreviewStateDefinition& definition) { return definition.shortcut == key; });
            if(stateIt != PREVIEW_STATES.end())
            {
                SetPreviewState(stateIt->state);
                return true;
            }
            break;
        }
        default: break;
    }
    return false;
}

void dskIngameUiPreview::SetPreviewState(const PreviewState state)
{
    previewState_ = state;
    UpdatePreviewStateControls();
}

void dskIngameUiPreview::CyclePreviewState(const int direction)
{
    const int stateCount = static_cast<int>(PreviewState::Count);
    const int currentIndex = static_cast<int>(GetPreviewStateIndex(previewState_));
    const int nextIndex = (currentIndex + direction + stateCount) % stateCount;
    SetPreviewState(PREVIEW_STATES[static_cast<std::size_t>(nextIndex)].state);
}

void dskIngameUiPreview::LayoutControls()
{
    const Extent size = GetSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const int centerX = width / 2;
    const int rightPanelLeft = std::max(620, width - 360);
    const int bottomBarTop = std::max(WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 280, height - BOTTOM_HUD_HEIGHT);

    GetCtrl<ctrlText>(ID_txtTitle)->SetPos(DrawPoint(centerX, 16));
    GetCtrl<ctrlText>(ID_txtNotice)->SetPos(DrawPoint(centerX, 39));
    GetCtrl<ctrlText>(ID_txtSize)->SetPos(DrawPoint(16, 18));
    GetCtrl<ctrlText>(ID_txtGuiScale)->SetPos(DrawPoint(16, 37));
    GetCtrl<ctrlText>(ID_txtFiltering)->SetPos(DrawPoint(16, 56));
    GetCtrl<ctrlText>(ID_txtActiveState)->SetPos(DrawPoint(centerX, 61));
    GetCtrl<ctrlText>(ID_txtStateHint)->SetPos(DrawPoint(centerX, 82));

    constexpr int stateButtonWidth = 126;
    constexpr int stateButtonGap = 8;
    constexpr int stateButtonColumns = 6;
    const int stateGridWidth = stateButtonColumns * stateButtonWidth + (stateButtonColumns - 1) * stateButtonGap;
    const DrawPoint stateButtonBase(centerX - stateGridWidth / 2, 94);
    for(std::size_t i = 0; i < PREVIEW_STATES.size(); ++i)
    {
        GetCtrl<ctrlButton>(PREVIEW_STATES[i].buttonId)
          ->SetPos(stateButtonBase
                   + DrawPoint(static_cast<int>(i % stateButtonColumns) * (stateButtonWidth + stateButtonGap),
                               static_cast<int>(i / stateButtonColumns) * 22));
    }

    const int dataTop = WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 18;
    GetCtrl<ctrlText>(ID_txtHudDataSource)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop));
    GetCtrl<ctrlText>(ID_txtHudCompactData)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop + 20));
    GetCtrl<ctrlText>(ID_txtHudResourceData)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop + 40));
    GetCtrl<ctrlText>(ID_txtHudMessageData)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop + 60));
    GetCtrl<ctrlText>(ID_txtHudContractData)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop + 80));
    GetCtrl<ctrlText>(ID_txtHudAvailabilityData)->SetPos(DrawPoint(rightPanelLeft + 18, dataTop + 100));

    for(unsigned i = 0; i < 8; ++i)
        GetCtrl<ctrlText>(ID_txtStateLine0 + i)->SetPos(DrawPoint(18, dataTop + static_cast<int>(i) * 20));

    const std::array<unsigned, 7> toggleButtons = {{ID_btToggleBuild, ID_btToggleRoads, ID_btToggleMilitary,
                                                    ID_btToggleEconomy, ID_btToggleMessages, ID_btToggleMap,
                                                    ID_btToggleSelection}};
    const std::array<int, 7> toggleWidths = {{58, 58, 70, 70, 72, 50, 76}};
    int toggleWidth = 0;
    for(const int buttonWidth : toggleWidths)
        toggleWidth += buttonWidth + 6;
    toggleWidth -= 6;

    DrawPoint togglePos(centerX - toggleWidth / 2, bottomBarTop + 18);
    for(std::size_t i = 0; i < toggleButtons.size(); ++i)
    {
        GetCtrl<ctrlButton>(toggleButtons[i])->SetPos(togglePos);
        togglePos.x += toggleWidths[i] + 6;
    }

    GetCtrl<ctrlButton>(ID_btActionPrimary)->SetPos(DrawPoint(18, WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 176));
    GetCtrl<ctrlButton>(ID_btActionSecondary)->SetPos(DrawPoint(110, WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 176));
    GetCtrl<ctrlButton>(ID_btActionDisabled)->SetPos(DrawPoint(202, WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 176));
    GetCtrl<ctrlButton>(ID_btBack)->SetPos(DrawPoint(width - 140, height - 32));

    UpdateDiagnostics();
    UpdatePreviewStateControls();
}

void dskIngameUiPreview::UpdateDiagnostics()
{
    GetCtrl<ctrlText>(ID_txtSize)->SetText(GetPreviewSizeLabel());
    GetCtrl<ctrlText>(ID_txtGuiScale)->SetText(GetGuiScaleLabel());
    GetCtrl<ctrlText>(ID_txtFiltering)->SetText(GetTextureFilteringLabel());
    RefreshHudData();
    GetCtrl<ctrlText>(ID_txtHudDataSource)->SetText("HUD data source: " + hudData_.sourceLabel);
    GetCtrl<ctrlText>(ID_txtHudCompactData)
      ->SetText("Compact chips: " + hudData_.topBarChips[1] + " | " + hudData_.topBarChips[2] + " | "
                + hudData_.topBarChips[3] + " | " + hudData_.topBarChips[4]);
    GetCtrl<ctrlText>(ID_txtHudResourceData)
      ->SetText("Resources panel: " + hudData_.resourceChips[0].label + " " + hudData_.resourceChips[0].value + " | "
                + hudData_.resourceChips[1].label + " " + hudData_.resourceChips[1].value + " | "
                + hudData_.resourceChips[2].label + " " + hudData_.resourceChips[2].value + " | "
                + hudData_.resourceChips[3].label + " " + hudData_.resourceChips[3].value);
    GetCtrl<ctrlText>(ID_txtHudMessageData)
      ->SetText(hudData_.messageLane + " | " + hudData_.selectedSummary + " | " + hudData_.mapSummary);
    GetCtrl<ctrlText>(ID_txtHudContractData)
      ->SetText(hudData_.developmentExportSummary
                + " represented: resources, military, messages, selection, map, commands, economy");
    GetCtrl<ctrlText>(ID_txtHudAvailabilityData)
      ->SetText("Availability: resources.gold=" + GetAvailabilityLabel(hudData_, "resources", "resources.gold")
                + " | minimap.thumbnail=" + GetAvailabilityLabel(hudData_, "map", "minimap.thumbnail")
                + " | commands.dispatch=" + GetAvailabilityLabel(hudData_, "commands", "commands.dispatch"));
}

void dskIngameUiPreview::RefreshHudData()
{
    hudData_ = hudDataProvider_->GetViewModel();
}

void dskIngameUiPreview::UpdatePreviewStateControls()
{
    const PreviewStateDefinition& definition = GetPreviewStateDefinition(previewState_);
    GetCtrl<ctrlText>(ID_txtActiveState)->SetText("Active preview state: " + std::string(definition.label));

    for(std::size_t i = 0; i < definition.lines.size(); ++i)
        GetCtrl<ctrlText>(ID_txtStateLine0 + static_cast<unsigned>(i))->SetText(definition.lines[i]);

    for(const PreviewStateDefinition& stateDefinition : PREVIEW_STATES)
    {
        ctrlButton& button = *GetCtrl<ctrlButton>(stateDefinition.buttonId);
        const bool isActive = stateDefinition.state == previewState_;
        button.SetChecked(isActive);
        button.SetTexture(isActive ? TextureColor::Green2 : TextureColor::Grey);
    }

    const std::array<std::pair<unsigned, PreviewState>, 7> toggleButtons = {
      {{ID_btToggleBuild, PreviewState::ToggleBuildRoads},
       {ID_btToggleRoads, PreviewState::ToggleBuildRoads},
       {ID_btToggleMilitary, PreviewState::ToggleMilitaryEconomy},
       {ID_btToggleEconomy, PreviewState::ToggleMilitaryEconomy},
       {ID_btToggleMessages, PreviewState::MessagesMinimap},
       {ID_btToggleMap, PreviewState::MessagesMinimap},
       {ID_btToggleSelection, PreviewState::SelectionContext}}};
    for(const auto& toggleButton : toggleButtons)
    {
        ctrlButton& button = *GetCtrl<ctrlButton>(toggleButton.first);
        const bool isActive = toggleButton.second == previewState_;
        button.SetChecked(isActive);
        button.SetTexture(isActive ? TextureColor::Green2 : TextureColor::Grey);
    }

    ctrlTextButton& primaryButton = *GetCtrl<ctrlTextButton>(ID_btActionPrimary);
    ctrlTextButton& secondaryButton = *GetCtrl<ctrlTextButton>(ID_btActionSecondary);
    ctrlTextButton& disabledButton = *GetCtrl<ctrlTextButton>(ID_btActionDisabled);
    primaryButton.SetEnabled(true);
    secondaryButton.SetEnabled(true);
    disabledButton.SetEnabled(false);
    primaryButton.SetTexture(TextureColor::Green2);
    secondaryButton.SetTexture(TextureColor::Green1);
    disabledButton.SetTexture(TextureColor::Grey);

    switch(previewState_)
    {
        case PreviewState::ToggleBuildRoads:
            primaryButton.SetText("Road");
            secondaryButton.SetText("Flag");
            disabledButton.SetText("Disabled");
            break;
        case PreviewState::ToggleMilitaryEconomy:
            primaryButton.SetText("Readiness");
            secondaryButton.SetText("Resources");
            disabledButton.SetText("Reserve");
            break;
        case PreviewState::MessagesMinimap:
            primaryButton.SetText("Mute");
            secondaryButton.SetText("Zoom +");
            disabledButton.SetText("Real map");
            break;
        case PreviewState::SelectionContext:
            primaryButton.SetText("Inspect");
            secondaryButton.SetText("Center");
            disabledButton.SetText("Command");
            break;
        case PreviewState::SmallScreenStress:
            primaryButton.SetText("Build");
            secondaryButton.SetText("Map");
            disabledButton.SetText("Overflow");
            break;
        default:
            primaryButton.SetText("Road");
            secondaryButton.SetText("Flag");
            disabledButton.SetText("Disabled");
            break;
    }
}

void dskIngameUiPreview::DrawPreviewBackground() const
{
    const Extent size = GetSize();
    DrawRectangle(Rect(DrawPoint::all(0), size), 0xFF162015);

    constexpr unsigned tileSize = 32;
    for(unsigned y = WORKBENCH_HEADER_HEIGHT; y < size.y; y += tileSize)
    {
        for(unsigned x = 0; x < size.x; x += tileSize)
        {
            const bool alternate = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            DrawRectangle(Rect(DrawPoint(x, y), Extent(tileSize, tileSize)), alternate ? COLOR_MAP_A : COLOR_MAP_B);
        }
    }

    for(unsigned x = 0; x < size.x; x += 64)
        DrawLine(DrawPoint(x, WORKBENCH_HEADER_HEIGHT), DrawPoint(x, size.y), 1, COLOR_MAP_GRID);
    for(unsigned y = WORKBENCH_HEADER_HEIGHT; y < size.y; y += 48)
        DrawLine(DrawPoint(0, y), DrawPoint(size.x, y), 1, COLOR_MAP_GRID);
}

void dskIngameUiPreview::DrawIngameShell() const
{
    const Extent size = GetSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const int topBarTop = WORKBENCH_HEADER_HEIGHT;
    const int bottomBarTop = std::max(WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 280, height - BOTTOM_HUD_HEIGHT);
    const int centerX = width / 2;

    DrawRectangle(Rect(DrawPoint(0, 0), Extent(size.x, WORKBENCH_HEADER_HEIGHT)), COLOR_EARTH_DARK);
    DrawRectangle(Rect(DrawPoint(0, topBarTop), Extent(size.x, TOP_HUD_HEIGHT)), COLOR_EARTH_PANEL);
    DrawRectangle(Rect(DrawPoint(0, bottomBarTop), Extent(size.x, height - bottomBarTop)), COLOR_EARTH_PANEL);
    DrawLine(DrawPoint(0, topBarTop + TOP_HUD_HEIGHT), DrawPoint(width, topBarTop + TOP_HUD_HEIGHT), 2,
             COLOR_EARTH_EDGE_DIM);
    DrawLine(DrawPoint(0, bottomBarTop), DrawPoint(width, bottomBarTop), 2, COLOR_EARTH_EDGE_DIM);

    DrawChip(DrawPoint(18, topBarTop + 7), Extent(88, 24), hudData_.playerLabel, true);
    DrawChip(DrawPoint(114, topBarTop + 7), Extent(62, 24), hudData_.topBarChips[1]);
    DrawChip(DrawPoint(184, topBarTop + 7), Extent(62, 24), hudData_.topBarChips[2]);
    DrawChip(DrawPoint(254, topBarTop + 7), Extent(62, 24), hudData_.topBarChips[3]);
    DrawChip(DrawPoint(width - 94, topBarTop + 7), Extent(76, 24), hudData_.topBarChips[4], hudData_.messageCount > 0);

    const DrawPoint messagePos(centerX - 170, topBarTop + 8);
    DrawRectangle(Rect(messagePos, Extent(340, 22)), 0xCC4E4129);
    SmallFont->Draw(messagePos + DrawPoint(12, 6), "Latest: " + hudData_.messageLane, FontStyle{}, COLOR_YELLOW);

    DrawRectangle(Rect(DrawPoint(centerX - 260, bottomBarTop + 10), Extent(520, 38)), 0xBB2F2A1D);
    SmallFont->Draw(DrawPoint(centerX, bottomBarTop + 43), "toggle-panel bar / static preview only", FontStyle::CENTER,
                    COLOR_EARTH_EDGE);
}

void dskIngameUiPreview::DrawPreviewState() const
{
    const Extent size = GetSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const int centerX = width / 2;
    const int topContent = WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 18;
    const int bottomBarTop = std::max(WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 280, height - BOTTOM_HUD_HEIGHT);
    const int rightPanelLeft = std::max(620, width - 360);
    const DrawPoint mainPanelPos(18, topContent + 154);

    switch(previewState_)
    {
        case PreviewState::ClassicCompactHud:
        {
            DrawClassicPanel(DrawPoint(centerX - 230, topContent + 34), Extent(460, 122),
                             "Classic-inspired compact HUD direction");
            SmallFont->Draw(DrawPoint(centerX - 206, topContent + 72), "Small permanent values: soldiers, gold, swords",
                            FontStyle{}, COLOR_YELLOW);
            SmallFont->Draw(DrawPoint(centerX - 206, topContent + 96), "Toggle panels replace old main/submenu travel",
                            FontStyle{}, COLOR_YELLOW);
            SmallFont->Draw(DrawPoint(centerX - 206, topContent + 120), "Map remains dominant; details open on demand",
                            FontStyle{}, COLOR_YELLOW);
            DrawChip(DrawPoint(centerX - 198, topContent + 144), Extent(76, 24), "Build");
            DrawChip(DrawPoint(centerX - 114, topContent + 144), Extent(76, 24), "Roads");
            DrawChip(DrawPoint(centerX - 30, topContent + 144), Extent(76, 24), "Military");
            DrawChip(DrawPoint(centerX + 54, topContent + 144), Extent(76, 24), "Economy");
            DrawChip(DrawPoint(centerX + 138, topContent + 144), Extent(76, 24), "Messages");
            break;
        }
        case PreviewState::ToggleBuildRoads:
        {
            DrawClassicPanel(mainPanelPos, Extent(322, 164), "Build");
            SmallFont->Draw(mainPanelPos + DrawPoint(16, 42), "House category / compact actions", FontStyle{},
                            COLOR_YELLOW);
            const std::array<const char*, 8> buildActions = {
              {"Hut", "Wood", "Mine", "Farm", "HQ", "Store", "Road", "Cancel"}};
            DrawActionGrid(mainPanelPos + DrawPoint(18, 68), buildActions, 0, 7);

            DrawClassicPanel(DrawPoint(mainPanelPos.x + 340, mainPanelPos.y), Extent(292, 164), "Roads");
            SmallFont->Draw(DrawPoint(mainPanelPos.x + 356, mainPanelPos.y + 42),
                            "Flag and road tools / no command dispatch", FontStyle{}, COLOR_YELLOW);
            const std::array<const char*, 8> roadActions = {
              {"Road", "Flag", "Split", "Erase", "Plan", "Undo", "+", "-"}};
            DrawActionGrid(DrawPoint(mainPanelPos.x + 358, mainPanelPos.y + 68), roadActions, 0, 5);
            break;
        }
        case PreviewState::ToggleMilitaryEconomy:
        {
            DrawClassicPanel(mainPanelPos, Extent(332, 178), "Military");
            SmallFont->Draw(mainPanelPos + DrawPoint(16, 42), hudData_.militarySummary, FontStyle{}, COLOR_YELLOW);
            SmallFont->Draw(mainPanelPos + DrawPoint(16, 66), "Readiness: Guarded | Capacity 184/240", FontStyle{},
                            COLOR_WARNING);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(18, 94), Extent(290, 12)), 0xFF6D9B3C);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(18, 118), Extent(70, 32)), 0xFF5B723A);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(98, 118), Extent(70, 32)), 0xFF5F8A40);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(178, 118), Extent(70, 32)), 0xFFD67B4A);

            DrawClassicPanel(DrawPoint(mainPanelPos.x + 350, mainPanelPos.y), Extent(306, 178), "Economy / Resources");
            for(unsigned i = 0; i < hudData_.resourceChips.size(); ++i)
            {
                const DrawPoint rowPos(mainPanelPos.x + 370, mainPanelPos.y + 44 + static_cast<int>(i) * 25);
                DrawRectangle(Rect(rowPos, Extent(252, 18)), i % 2 == 0 ? 0xAA4A4029 : 0xAA3B3322);
                SmallFont->Draw(rowPos + DrawPoint(10, 4),
                                hudData_.resourceChips[i].label + "  " + hudData_.resourceChips[i].value, FontStyle{},
                                COLOR_YELLOW);
            }
            SmallFont->Draw(DrawPoint(mainPanelPos.x + 370, mainPanelPos.y + 150),
                            "Storage pressure: not safely accessible yet", FontStyle{}, COLOR_WARNING);
            break;
        }
        case PreviewState::MessagesMinimap:
        {
            DrawClassicPanel(DrawPoint(centerX - 270, topContent + 18), Extent(540, 126), "Messages");
            DrawRectangle(Rect(DrawPoint(centerX - 250, topContent + 52), Extent(500, 24)), 0xCC6D4D26);
            SmallFont->Draw(DrawPoint(centerX - 236, topContent + 58),
                            "Toast: Storehouse cannot receive any more wares", FontStyle{}, COLOR_YELLOW);
            DrawChip(DrawPoint(centerX + 164, topContent + 54), Extent(64, 20), "Mute");
            SmallFont->Draw(DrawPoint(centerX - 236, topContent + 88),
                            "Unread: " + std::to_string(hudData_.messageCount)
                              + " | latest message label from messages group",
                            FontStyle{}, COLOR_YELLOW);

            DrawClassicPanel(DrawPoint(26, bottomBarTop - 174), Extent(210, 154), "Map bottom-left");
            DrawMiniMap(DrawPoint(44, bottomBarTop - 128), Extent(174, 82));
            DrawChip(DrawPoint(48, bottomBarTop - 38), Extent(48, 20), "Zoom+");
            DrawChip(DrawPoint(102, bottomBarTop - 38), Extent(48, 20), "Zoom-");
            DrawChip(DrawPoint(156, bottomBarTop - 38), Extent(58, 20), "Collapse");

            DrawClassicPanel(DrawPoint(26, topContent + 152), Extent(210, 136), "Map top-left");
            DrawMiniMap(DrawPoint(44, topContent + 194), Extent(174, 70));
            SmallFont->Draw(DrawPoint(52, topContent + 272), hudData_.mapSummary, FontStyle{}, COLOR_YELLOW);
            break;
        }
        case PreviewState::SelectionContext:
        {
            DrawClassicPanel(DrawPoint(centerX - 250, topContent + 54), Extent(500, 176), "Selection / Context");
            SmallFont->Draw(DrawPoint(centerX - 226, topContent + 92), hudData_.selectedSummary, FontStyle{},
                            COLOR_YELLOW);
            SmallFont->Draw(DrawPoint(centerX - 226, topContent + 116),
                            "Inventory snippets: Gold " + hudData_.resourceChips[0].value + " | Swords "
                              + hudData_.resourceChips[1].value + " | Food " + hudData_.resourceChips[2].value,
                            FontStyle{}, COLOR_YELLOW);
            const std::array<const char*, 8> contextActions = {
              {"View", "Center", "Goods", "Workers", "Pin", "Close", "Upgrade", "Stop"}};
            DrawActionGrid(DrawPoint(centerX - 226, topContent + 144), contextActions, 0, 7);
            SmallFont->Draw(DrawPoint(centerX - 226, topContent + 214),
                            "Context actions represented by commands group; dispatch disabled", FontStyle{},
                            COLOR_WARNING);
            break;
        }
        case PreviewState::SmallScreenStress:
        {
            DrawRectangle(Rect(DrawPoint(centerX - 320, topContent + 12), Extent(640, 360)), 0x3020A0FF);
            DrawLine(DrawPoint(centerX - 320, topContent + 12), DrawPoint(centerX + 320, topContent + 372), 1,
                     COLOR_EARTH_EDGE_DIM);
            DrawLine(DrawPoint(centerX + 320, topContent + 12), DrawPoint(centerX - 320, topContent + 372), 1,
                     COLOR_EARTH_EDGE_DIM);
            SmallFont->Draw(DrawPoint(centerX, topContent + 24), "1280x720 pressure box / reduced state stress",
                            FontStyle::CENTER, COLOR_YELLOW);
            DrawClassicPanel(DrawPoint(centerX - 292, topContent + 54), Extent(196, 92), "Toast risk");
            SmallFont->Draw(DrawPoint(centerX - 274, topContent + 92), "messages vs top-left map", FontStyle{},
                            COLOR_YELLOW);
            DrawClassicPanel(DrawPoint(centerX + 82, topContent + 66), Extent(194, 116), "Context risk");
            SmallFont->Draw(DrawPoint(centerX + 100, topContent + 104), "panel width vs values", FontStyle{},
                            COLOR_YELLOW);
            DrawClassicPanel(DrawPoint(centerX - 292, bottomBarTop - 166), Extent(206, 132), "Minimap risk");
            DrawMiniMap(DrawPoint(centerX - 274, bottomBarTop - 120), Extent(168, 72));
            DrawRectangle(Rect(DrawPoint(centerX - 180, bottomBarTop - 36), Extent(360, 28)), 0x80D85038);
            SmallFont->Draw(DrawPoint(centerX, bottomBarTop - 29), "bottom toggle density / text clipping risk",
                            FontStyle::CENTER, COLOR_YELLOW);
            break;
        }
        case PreviewState::Count: break;
    }

    DrawClassicPanel(DrawPoint(rightPanelLeft + 10, topContent - 8), Extent(width - rightPanelLeft - 20, 132),
                     "Data contract");
}
