// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "dskIngameUiPreview.h"
#include "Loader.h"
#include "Settings.h"
#include "TextureFiltering.h"
#include "Window.h"
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
constexpr unsigned COLOR_ICON_INK = 0xFF171109;
constexpr unsigned COLOR_MUTED_MARKER = 0xAA9D8154;

enum class PreviewGlyph
{
    Player,
    Soldier,
    Gold,
    Sword,
    Post,
    Food,
    Coins,
    Boards,
    Stones,
    Build,
    Road,
    Military,
    Economy,
    Map,
    Selection,
    Disabled
};

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
      "1 HUD",
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
      "2 B/R",
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
      "3 M/E",
      U'3',
      "Toggle Panels: Military/Economy",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Military panel uses military group: soldiers, ranks, armor; status/capacity stay unavailable",
        "Economy panel uses resources group: gold, swords, food, coins, boards, stones",
        "Important values are readable but intentionally small enough to keep the map dominant",
        "Mock fields are visible; unavailable economy summaries stay marked not safely accessible yet",
        "No barracks, production, combat, inventory mutation, or simulation logic is queried",
        "Later UI question: which values deserve permanent chips versus on-demand panels?",
        "Classic strategy-game mood with cleaner grouping and lighter frame weight"}}},
     {dskIngameUiPreview::PreviewState::MessagesMinimap,
      ID_btStateMessagesMinimap,
      "4 P/Map",
      U'4',
      "Messages + Minimap",
      {{"Developer Preview | Static mock data | Not product UI | No gameplay logic",
        "Messages panel uses messages group: unread count; latest message label is unavailable",
        "Small toast preview is visible without becoming a blocking modal window",
        "Map panel uses map group: map size plus static minimap placeholder card",
        "Minimap placement previews bottom-left and top-left conflict zones in one reduced state",
        "Zoom + / Zoom - / collapse / expand are static preview controls only",
        "No real map data, fog, viewport, GameWorldView minimap renderer, or CONFIG.INI changes",
        "Later UI question: should minimap be docked, toggled, or collapsible by default?"}}},
     {dskIngameUiPreview::PreviewState::SelectionContext,
      ID_btStateSelectionContext,
      "5 Sel",
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
      "6 Stress",
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

std::string PlayerChipValue(const std::string& playerLabel)
{
    const std::string::size_type space = playerLabel.find(' ');
    return space == std::string::npos ? playerLabel : playerLabel.substr(0, space);
}

std::string MapSizeValue(const std::string& mapSummary)
{
    const std::string::size_type prefixSize = 4;
    const std::string::size_type delimiter = mapSummary.find(" |");
    if(mapSummary.size() <= prefixSize)
        return mapSummary;
    return mapSummary.substr(prefixSize, delimiter == std::string::npos ? std::string::npos : delimiter - prefixSize);
}

PreviewGlyph ResourceGlyph(const std::string& icon)
{
    if(icon == "Au")
        return PreviewGlyph::Gold;
    if(icon == "Sw")
        return PreviewGlyph::Sword;
    if(icon == "Fd")
        return PreviewGlyph::Food;
    if(icon == "Co")
        return PreviewGlyph::Coins;
    if(icon == "Bd")
        return PreviewGlyph::Boards;
    if(icon == "St")
        return PreviewGlyph::Stones;
    return PreviewGlyph::Disabled;
}

void DrawPreviewGlyph(const DrawPoint pos, const PreviewGlyph glyph, const unsigned color)
{
    Window::DrawRectangle(Rect(pos + DrawPoint(1, 1), Extent(14, 14)), COLOR_ICON_INK);
    switch(glyph)
    {
        case PreviewGlyph::Player:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 3), Extent(2, 10)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 3), Extent(7, 5)), color);
            Window::DrawLine(pos + DrawPoint(6, 8), pos + DrawPoint(12, 11), 1, color);
            break;
        case PreviewGlyph::Soldier:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 3), Extent(8, 9)), color);
            Window::DrawLine(pos + DrawPoint(4, 12), pos + DrawPoint(8, 15), 1, color);
            Window::DrawLine(pos + DrawPoint(12, 12), pos + DrawPoint(8, 15), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 5), Extent(2, 6)), COLOR_ICON_INK);
            break;
        case PreviewGlyph::Gold:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(8, 8)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 2), Extent(6, 3)), SetAlpha(color, 210));
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 10), Extent(10, 3)), SetAlpha(color, 180));
            break;
        case PreviewGlyph::Sword:
            Window::DrawLine(pos + DrawPoint(4, 12), pos + DrawPoint(12, 4), 2, color);
            Window::DrawLine(pos + DrawPoint(5, 9), pos + DrawPoint(8, 12), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 12), Extent(3, 3)), color);
            break;
        case PreviewGlyph::Post:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 5), Extent(10, 8)), color);
            Window::DrawLine(pos + DrawPoint(3, 5), pos + DrawPoint(8, 10), 1, COLOR_ICON_INK);
            Window::DrawLine(pos + DrawPoint(13, 5), pos + DrawPoint(8, 10), 1, COLOR_ICON_INK);
            break;
        case PreviewGlyph::Food:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(3, 8)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(8, 3), Extent(2, 10)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(11, 5), Extent(2, 7)), color);
            break;
        case PreviewGlyph::Coins:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(9, 3)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 7), Extent(9, 3)), SetAlpha(color, 210));
            Window::DrawRectangle(Rect(pos + DrawPoint(5, 10), Extent(8, 3)), SetAlpha(color, 180));
            break;
        case PreviewGlyph::Boards:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 4), Extent(10, 3)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 8), Extent(10, 3)), SetAlpha(color, 220));
            Window::DrawRectangle(Rect(pos + DrawPoint(2, 12), Extent(10, 2)), SetAlpha(color, 190));
            break;
        case PreviewGlyph::Stones:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 8), Extent(5, 5)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(8, 5), Extent(5, 5)), SetAlpha(color, 220));
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 11), Extent(7, 3)), SetAlpha(color, 190));
            break;
        case PreviewGlyph::Build:
            Window::DrawLine(pos + DrawPoint(3, 8), pos + DrawPoint(8, 3), 1, color);
            Window::DrawLine(pos + DrawPoint(8, 3), pos + DrawPoint(13, 8), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(5, 8), Extent(7, 6)), color);
            break;
        case PreviewGlyph::Road:
            Window::DrawLine(pos + DrawPoint(3, 12), pos + DrawPoint(13, 4), 2, color);
            Window::DrawLine(pos + DrawPoint(5, 14), pos + DrawPoint(15, 6), 1, SetAlpha(color, 180));
            break;
        case PreviewGlyph::Military:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(8, 8)), color);
            Window::DrawLine(pos + DrawPoint(4, 8), pos + DrawPoint(12, 8), 1, COLOR_ICON_INK);
            Window::DrawLine(pos + DrawPoint(8, 4), pos + DrawPoint(8, 12), 1, COLOR_ICON_INK);
            break;
        case PreviewGlyph::Economy:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 10), Extent(3, 4)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 7), Extent(3, 7)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(11, 4), Extent(3, 10)), color);
            break;
        case PreviewGlyph::Map:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 4), Extent(10, 8)), color);
            Window::DrawLine(pos + DrawPoint(6, 4), pos + DrawPoint(6, 12), 1, COLOR_ICON_INK);
            Window::DrawLine(pos + DrawPoint(10, 4), pos + DrawPoint(10, 12), 1, COLOR_ICON_INK);
            break;
        case PreviewGlyph::Selection:
            Window::DrawLine(pos + DrawPoint(8, 3), pos + DrawPoint(8, 13), 1, color);
            Window::DrawLine(pos + DrawPoint(3, 8), pos + DrawPoint(13, 8), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 6), Extent(4, 4)), color);
            break;
        case PreviewGlyph::Disabled:
            Window::DrawLine(pos + DrawPoint(4, 4), pos + DrawPoint(12, 12), 2, color);
            Window::DrawLine(pos + DrawPoint(12, 4), pos + DrawPoint(4, 12), 2, color);
            break;
    }
}

void DrawIconValueChip(const DrawPoint pos, const Extent size, const PreviewGlyph glyph, const std::string& value,
                       const unsigned accent, const bool active = false, const std::string& marker = "")
{
    Window::DrawRectangle(Rect(pos, size), active ? COLOR_CHIP_ACTIVE : COLOR_CHIP);
    Window::DrawRectangle(Rect(pos + DrawPoint(2, 2), Extent(size.x - 4, size.y - 4)),
                          active ? 0xCC6A5630 : 0xCC463A25);
    Window::DrawLine(pos + DrawPoint(4, size.y - 2), pos + DrawPoint(size.x - 4, size.y - 2), 1, accent);
    DrawPreviewGlyph(pos + DrawPoint(5, 4), glyph, accent);
    SmallFont->Draw(pos + DrawPoint(size.x - 7, 6), value, FontStyle::RIGHT, COLOR_YELLOW);
    if(!marker.empty())
        SmallFont->Draw(pos + DrawPoint(size.x - 4, size.y - 11), marker, FontStyle::RIGHT, COLOR_MUTED_MARKER);
}

void DrawIconButtonPreview(const DrawPoint pos, const PreviewGlyph glyph, const bool active,
                           const bool disabled = false)
{
    const unsigned accent = disabled ? COLOR_GREY : (active ? COLOR_WARNING : COLOR_EARTH_EDGE);
    Window::DrawRectangle(Rect(pos, Extent(28, 22)),
                          disabled ? COLOR_DISABLED : (active ? COLOR_CHIP_ACTIVE : COLOR_CHIP));
    DrawPreviewGlyph(pos + DrawPoint(6, 3), glyph, accent);
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
        SmallFont->Draw(buttonPos + DrawPoint(58, 15), "plh", FontStyle::RIGHT, COLOR_MUTED_MARKER);
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
    AddText(ID_txtNotice, DrawPoint(0, 0), "Developer preview | mock data | product UI and gameplay untouched",
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

    AddTextButton(ID_btToggleBuild, DrawPoint(0, 0), Extent(34, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleRoads, DrawPoint(0, 0), Extent(34, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleMilitary, DrawPoint(0, 0), Extent(34, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleEconomy, DrawPoint(0, 0), Extent(34, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleMessages, DrawPoint(0, 0), Extent(34, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleMap, DrawPoint(0, 0), Extent(42, 24), TextureColor::Grey, "", SmallFont);
    AddTextButton(ID_btToggleSelection, DrawPoint(0, 0), Extent(42, 24), TextureColor::Grey, "", SmallFont);

    AddTextButton(ID_btActionPrimary, DrawPoint(0, 0), Extent(56, 22), TextureColor::Green2, "", SmallFont);
    AddTextButton(ID_btActionSecondary, DrawPoint(0, 0), Extent(56, 22), TextureColor::Green1, "", SmallFont);
    AddTextButton(ID_btActionDisabled, DrawPoint(0, 0), Extent(56, 22), TextureColor::Grey, "", SmallFont);
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

void dskIngameUiPreview::Msg_PaintAfter()
{
    Desktop::Msg_PaintAfter();

    const std::array<std::pair<unsigned, PreviewGlyph>, 7> toggleGlyphs = {
      {{ID_btToggleBuild, PreviewGlyph::Build},
       {ID_btToggleRoads, PreviewGlyph::Road},
       {ID_btToggleMilitary, PreviewGlyph::Military},
       {ID_btToggleEconomy, PreviewGlyph::Economy},
       {ID_btToggleMessages, PreviewGlyph::Post},
       {ID_btToggleMap, PreviewGlyph::Map},
       {ID_btToggleSelection, PreviewGlyph::Selection}}};
    for(const auto& toggleGlyph : toggleGlyphs)
    {
        const ctrlButton& button = *GetCtrl<ctrlButton>(toggleGlyph.first);
        const Extent buttonSize = button.GetSize();
        const DrawPoint glyphPos = button.GetDrawPos() + DrawPoint(static_cast<int>(buttonSize.x - 16) / 2, 4);
        DrawPreviewGlyph(glyphPos, toggleGlyph.second, button.GetCheck() ? COLOR_WARNING : COLOR_EARTH_EDGE);
    }

    PreviewGlyph primaryGlyph = PreviewGlyph::Road;
    PreviewGlyph secondaryGlyph = PreviewGlyph::Build;
    switch(previewState_)
    {
        case PreviewState::ToggleMilitaryEconomy:
            primaryGlyph = PreviewGlyph::Military;
            secondaryGlyph = PreviewGlyph::Economy;
            break;
        case PreviewState::MessagesMinimap:
            primaryGlyph = PreviewGlyph::Post;
            secondaryGlyph = PreviewGlyph::Map;
            break;
        case PreviewState::SelectionContext:
            primaryGlyph = PreviewGlyph::Selection;
            secondaryGlyph = PreviewGlyph::Map;
            break;
        case PreviewState::SmallScreenStress:
            primaryGlyph = PreviewGlyph::Build;
            secondaryGlyph = PreviewGlyph::Map;
            break;
        default: break;
    }

    const std::array<std::pair<unsigned, PreviewGlyph>, 3> actionGlyphs = {
      {{ID_btActionPrimary, primaryGlyph},
       {ID_btActionSecondary, secondaryGlyph},
       {ID_btActionDisabled, PreviewGlyph::Disabled}}};
    for(const auto& actionGlyph : actionGlyphs)
    {
        const ctrlButton& button = *GetCtrl<ctrlButton>(actionGlyph.first);
        const Extent buttonSize = button.GetSize();
        const DrawPoint glyphPos = button.GetDrawPos() + DrawPoint(static_cast<int>(buttonSize.x - 16) / 2, 3);
        DrawPreviewGlyph(glyphPos, actionGlyph.second, button.GetEnabled() ? COLOR_EARTH_EDGE : COLOR_GREY);
    }
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
    const std::array<int, 7> toggleWidths = {{34, 34, 34, 34, 34, 42, 42}};
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
    GetCtrl<ctrlButton>(ID_btActionSecondary)->SetPos(DrawPoint(80, WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 176));
    GetCtrl<ctrlButton>(ID_btActionDisabled)->SetPos(DrawPoint(142, WORKBENCH_HEADER_HEIGHT + TOP_HUD_HEIGHT + 176));
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
                + hudData_.resourceChips[3].label + " " + hudData_.resourceChips[3].value + " | "
                + hudData_.resourceChips[4].label + " " + hudData_.resourceChips[4].value + " | "
                + hudData_.resourceChips[5].label + " " + hudData_.resourceChips[5].value);
    GetCtrl<ctrlText>(ID_txtHudMessageData)
      ->SetText(hudData_.messageLane + " | " + hudData_.selectedSummary + " | " + hudData_.mapSummary);
    GetCtrl<ctrlText>(ID_txtHudContractData)
      ->SetText(hudData_.developmentExportSummary
                + " represented: player, military, resources, messages, selection, map, commands, economy");
    GetCtrl<ctrlText>(ID_txtHudAvailabilityData)
      ->SetText("Markers: " + hudData_.sourceLabel
                + " | resources.gold=" + GetAvailabilityLabel(hudData_, "resources", "resources.gold")
                + " | minimap.thumbnail=" + GetAvailabilityLabel(hudData_, "map", "minimap.thumbnail") + " | latest="
                + GetAvailabilityLabel(hudData_, "messages", "messages.latest.label") + " | command eligibility="
                + GetAvailabilityLabel(hudData_, "commands", "commands.quick.build.eligibility"));
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
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
            break;
        case PreviewState::ToggleMilitaryEconomy:
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
            break;
        case PreviewState::MessagesMinimap:
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
            break;
        case PreviewState::SelectionContext:
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
            break;
        case PreviewState::SmallScreenStress:
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
            break;
        default:
            primaryButton.SetText("");
            secondaryButton.SetText("");
            disabledButton.SetText("");
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

    DrawIconValueChip(DrawPoint(18, topBarTop + 7), Extent(58, 24), PreviewGlyph::Player,
                      PlayerChipValue(hudData_.playerLabel), COLOR_EARTH_EDGE, true);
    DrawIconValueChip(DrawPoint(84, topBarTop + 7), Extent(58, 24), PreviewGlyph::Soldier,
                      std::to_string(hudData_.totalSoldiers), COLOR_WARNING);
    DrawIconValueChip(DrawPoint(150, topBarTop + 7), Extent(58, 24), PreviewGlyph::Gold,
                      hudData_.resourceChips[0].value, COLOR_EARTH_EDGE);
    DrawIconValueChip(DrawPoint(216, topBarTop + 7), Extent(58, 24), PreviewGlyph::Sword,
                      hudData_.resourceChips[1].value, COLOR_EARTH_EDGE);
    DrawIconValueChip(DrawPoint(width - 76, topBarTop + 7), Extent(58, 24), PreviewGlyph::Post,
                      std::to_string(hudData_.messageCount),
                      hudData_.messageCount > 0 ? COLOR_WARNING : COLOR_EARTH_EDGE, hudData_.messageCount > 0);

    DrawRectangle(Rect(DrawPoint(centerX - 164, bottomBarTop + 10), Extent(328, 38)), 0xBB2F2A1D);
    const std::array<PreviewGlyph, 7> toggleGlyphs = {{PreviewGlyph::Build, PreviewGlyph::Road, PreviewGlyph::Military,
                                                       PreviewGlyph::Economy, PreviewGlyph::Post, PreviewGlyph::Map,
                                                       PreviewGlyph::Selection}};
    const std::array<PreviewState, 7> toggleStates = {
      {PreviewState::ToggleBuildRoads, PreviewState::ToggleBuildRoads, PreviewState::ToggleMilitaryEconomy,
       PreviewState::ToggleMilitaryEconomy, PreviewState::MessagesMinimap, PreviewState::MessagesMinimap,
       PreviewState::SelectionContext}};
    for(unsigned i = 0; i < toggleGlyphs.size(); ++i)
    {
        const DrawPoint iconPos(centerX - 144 + static_cast<int>(i) * 42, bottomBarTop + 18);
        DrawIconButtonPreview(iconPos, toggleGlyphs[i], toggleStates[i] == previewState_);
    }
    SmallFont->Draw(DrawPoint(centerX + 158, bottomBarTop + 32), "no cmd", FontStyle{}, COLOR_MUTED_MARKER);
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
                             "Icon-first compact HUD direction");
            DrawIconValueChip(DrawPoint(centerX - 190, topContent + 70), Extent(58, 24), PreviewGlyph::Soldier,
                              std::to_string(hudData_.totalSoldiers), COLOR_WARNING);
            DrawIconValueChip(DrawPoint(centerX - 124, topContent + 70), Extent(58, 24), PreviewGlyph::Gold,
                              hudData_.resourceChips[0].value, COLOR_EARTH_EDGE);
            DrawIconValueChip(DrawPoint(centerX - 58, topContent + 70), Extent(58, 24), PreviewGlyph::Sword,
                              hudData_.resourceChips[1].value, COLOR_EARTH_EDGE);
            DrawIconValueChip(DrawPoint(centerX + 8, topContent + 70), Extent(58, 24), PreviewGlyph::Post,
                              std::to_string(hudData_.messageCount), COLOR_WARNING, hudData_.messageCount > 0);
            DrawIconValueChip(DrawPoint(centerX + 74, topContent + 70), Extent(86, 24), PreviewGlyph::Map,
                              MapSizeValue(hudData_.mapSummary), COLOR_EARTH_EDGE);
            SmallFont->Draw(DrawPoint(centerX - 190, topContent + 112),
                            "Labels stay in panels; permanent bars carry symbols and counts", FontStyle{},
                            COLOR_YELLOW);
            SmallFont->Draw(DrawPoint(centerX - 190, topContent + 136), "mock/live markers stay subdued", FontStyle{},
                            COLOR_MUTED_MARKER);
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
            SmallFont->Draw(mainPanelPos + DrawPoint(16, 66), "Status/capacity: not safely accessible yet", FontStyle{},
                            COLOR_WARNING);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(18, 94), Extent(290, 12)), 0xFF6D9B3C);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(18, 118), Extent(70, 32)), 0xFF5B723A);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(98, 118), Extent(70, 32)), 0xFF5F8A40);
            DrawRectangle(Rect(mainPanelPos + DrawPoint(178, 118), Extent(70, 32)), 0xFFD67B4A);

            DrawClassicPanel(DrawPoint(mainPanelPos.x + 350, mainPanelPos.y), Extent(306, 178), "Economy / Resources");
            for(unsigned i = 0; i < hudData_.resourceChips.size(); ++i)
            {
                const DrawPoint rowPos(mainPanelPos.x + 370 + static_cast<int>(i % 2) * 130,
                                       mainPanelPos.y + 44 + static_cast<int>(i / 2) * 25);
                DrawIconValueChip(rowPos, Extent(92, 20), ResourceGlyph(hudData_.resourceChips[i].icon),
                                  hudData_.resourceChips[i].value, i == 2 ? COLOR_WARNING : COLOR_EARTH_EDGE);
            }
            SmallFont->Draw(DrawPoint(mainPanelPos.x + 370, mainPanelPos.y + 130),
                            "Storage pressure: not safely accessible yet", FontStyle{}, COLOR_WARNING);
            break;
        }
        case PreviewState::MessagesMinimap:
        {
            DrawClassicPanel(DrawPoint(centerX - 270, topContent + 18), Extent(540, 126), "Messages");
            DrawRectangle(Rect(DrawPoint(centerX - 250, topContent + 52), Extent(500, 24)), 0xCC6D4D26);
            SmallFont->Draw(DrawPoint(centerX - 236, topContent + 58),
                            "Toast: latest message text not safely accessible yet", FontStyle{}, COLOR_WARNING);
            DrawIconValueChip(DrawPoint(centerX + 164, topContent + 54), Extent(58, 20), PreviewGlyph::Post,
                              std::to_string(hudData_.messageCount), COLOR_WARNING, hudData_.messageCount > 0);
            SmallFont->Draw(DrawPoint(centerX - 236, topContent + 88),
                            "Unread: " + std::to_string(hudData_.messageCount) + " | latest label stays unavailable",
                            FontStyle{}, COLOR_YELLOW);

            DrawClassicPanel(DrawPoint(26, bottomBarTop - 174), Extent(210, 154), "Map bottom-left");
            DrawMiniMap(DrawPoint(44, bottomBarTop - 128), Extent(174, 82));
            DrawIconValueChip(DrawPoint(48, bottomBarTop - 38), Extent(48, 20), PreviewGlyph::Map, "+",
                              COLOR_EARTH_EDGE, false, "plh");
            DrawIconValueChip(DrawPoint(102, bottomBarTop - 38), Extent(48, 20), PreviewGlyph::Map, "-",
                              COLOR_EARTH_EDGE, false, "plh");
            DrawIconValueChip(DrawPoint(156, bottomBarTop - 38), Extent(58, 20), PreviewGlyph::Disabled, "", COLOR_GREY,
                              false, "plh");

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
            DrawIconValueChip(DrawPoint(centerX - 226, topContent + 116), Extent(70, 20), PreviewGlyph::Gold,
                              hudData_.resourceChips[0].value, COLOR_EARTH_EDGE);
            DrawIconValueChip(DrawPoint(centerX - 148, topContent + 116), Extent(70, 20), PreviewGlyph::Sword,
                              hudData_.resourceChips[1].value, COLOR_EARTH_EDGE);
            DrawIconValueChip(DrawPoint(centerX - 70, topContent + 116), Extent(70, 20), PreviewGlyph::Boards,
                              hudData_.resourceChips[4].value, COLOR_EARTH_EDGE);
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
