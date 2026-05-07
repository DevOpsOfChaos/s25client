// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "DeveloperHudOverlay.h"
#include "DeveloperHudDataProvider.h"
#include "Loader.h"
#include "Window.h"
#include "ogl/FontStyle.h"
#include "ogl/glFont.h"
#include "s25util/colors.h"
#include <algorithm>
#include <array>
#include <string>

namespace {
constexpr unsigned COLOR_PANEL = 0xDD211B13;
constexpr unsigned COLOR_PANEL_ALT = 0xCC3F3422;
constexpr unsigned COLOR_ACCENT = 0xFFE3C681;
constexpr unsigned COLOR_GOOD = 0xFFA9B86C;
constexpr unsigned COLOR_WARNING = 0xFFD67B4A;
constexpr unsigned COLOR_DIM = 0xCC201A12;
constexpr unsigned COLOR_INK = 0xFF15100A;
constexpr unsigned COLOR_MUTED = 0xAA9D8154;

enum class HudGlyph
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
    Alert
};

void DrawSoftPanel(const Rect& rect, const unsigned color)
{
    const DrawPoint pos = rect.getOrigin();
    const Extent size = rect.getSize();
    Window::DrawRectangle(Rect(pos + DrawPoint(4, 0), Extent(size.x - 8, size.y)), color);
    Window::DrawRectangle(Rect(pos + DrawPoint(0, 4), Extent(size.x, size.y - 8)), color);
}

std::string PlayerChipValue(const std::string& playerLabel)
{
    const std::string::size_type space = playerLabel.find(' ');
    return space == std::string::npos ? playerLabel : playerLabel.substr(0, space);
}

HudGlyph ResourceGlyph(const std::string& icon)
{
    if(icon == "Au")
        return HudGlyph::Gold;
    if(icon == "Sw")
        return HudGlyph::Sword;
    if(icon == "Fd")
        return HudGlyph::Food;
    if(icon == "Co")
        return HudGlyph::Coins;
    if(icon == "Bd")
        return HudGlyph::Boards;
    if(icon == "St")
        return HudGlyph::Stones;
    return HudGlyph::Alert;
}

void DrawGlyph(const DrawPoint pos, const HudGlyph glyph, const unsigned color)
{
    Window::DrawRectangle(Rect(pos + DrawPoint(1, 1), Extent(14, 14)), COLOR_INK);
    switch(glyph)
    {
        case HudGlyph::Player:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 3), Extent(2, 10)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 3), Extent(7, 5)), color);
            Window::DrawLine(pos + DrawPoint(6, 8), pos + DrawPoint(12, 11), 1, color);
            break;
        case HudGlyph::Soldier:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 3), Extent(8, 9)), color);
            Window::DrawLine(pos + DrawPoint(4, 12), pos + DrawPoint(8, 15), 1, color);
            Window::DrawLine(pos + DrawPoint(12, 12), pos + DrawPoint(8, 15), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 5), Extent(2, 6)), COLOR_INK);
            break;
        case HudGlyph::Gold:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(8, 8)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 2), Extent(6, 3)), SetAlpha(color, 210));
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 10), Extent(10, 3)), SetAlpha(color, 180));
            break;
        case HudGlyph::Sword:
            Window::DrawLine(pos + DrawPoint(4, 12), pos + DrawPoint(12, 4), 2, color);
            Window::DrawLine(pos + DrawPoint(5, 9), pos + DrawPoint(8, 12), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 12), Extent(3, 3)), color);
            break;
        case HudGlyph::Post:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 5), Extent(10, 8)), color);
            Window::DrawLine(pos + DrawPoint(3, 5), pos + DrawPoint(8, 10), 1, COLOR_INK);
            Window::DrawLine(pos + DrawPoint(13, 5), pos + DrawPoint(8, 10), 1, COLOR_INK);
            break;
        case HudGlyph::Food:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(3, 8)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(8, 3), Extent(2, 10)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(11, 5), Extent(2, 7)), color);
            break;
        case HudGlyph::Coins:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(9, 3)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 7), Extent(9, 3)), SetAlpha(color, 210));
            Window::DrawRectangle(Rect(pos + DrawPoint(5, 10), Extent(8, 3)), SetAlpha(color, 180));
            break;
        case HudGlyph::Boards:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 4), Extent(10, 3)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 8), Extent(10, 3)), SetAlpha(color, 220));
            Window::DrawRectangle(Rect(pos + DrawPoint(2, 12), Extent(10, 2)), SetAlpha(color, 190));
            break;
        case HudGlyph::Stones:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 8), Extent(5, 5)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(8, 5), Extent(5, 5)), SetAlpha(color, 220));
            Window::DrawRectangle(Rect(pos + DrawPoint(6, 11), Extent(7, 3)), SetAlpha(color, 190));
            break;
        case HudGlyph::Build:
            Window::DrawLine(pos + DrawPoint(3, 8), pos + DrawPoint(8, 3), 1, color);
            Window::DrawLine(pos + DrawPoint(8, 3), pos + DrawPoint(13, 8), 1, color);
            Window::DrawRectangle(Rect(pos + DrawPoint(5, 8), Extent(7, 6)), color);
            break;
        case HudGlyph::Road:
            Window::DrawLine(pos + DrawPoint(3, 12), pos + DrawPoint(13, 4), 2, color);
            Window::DrawLine(pos + DrawPoint(5, 14), pos + DrawPoint(15, 6), 1, SetAlpha(color, 180));
            break;
        case HudGlyph::Military:
            Window::DrawRectangle(Rect(pos + DrawPoint(4, 4), Extent(8, 8)), color);
            Window::DrawLine(pos + DrawPoint(4, 8), pos + DrawPoint(12, 8), 1, COLOR_INK);
            Window::DrawLine(pos + DrawPoint(8, 4), pos + DrawPoint(8, 12), 1, COLOR_INK);
            break;
        case HudGlyph::Economy:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 10), Extent(3, 4)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 7), Extent(3, 7)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(11, 4), Extent(3, 10)), color);
            break;
        case HudGlyph::Map:
            Window::DrawRectangle(Rect(pos + DrawPoint(3, 4), Extent(10, 8)), color);
            Window::DrawLine(pos + DrawPoint(6, 4), pos + DrawPoint(6, 12), 1, COLOR_INK);
            Window::DrawLine(pos + DrawPoint(10, 4), pos + DrawPoint(10, 12), 1, COLOR_INK);
            break;
        case HudGlyph::Alert:
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 4), Extent(2, 7)), color);
            Window::DrawRectangle(Rect(pos + DrawPoint(7, 13), Extent(2, 2)), color);
            break;
    }
}

void DrawIconValueChip(const DrawPoint pos, const Extent size, const HudGlyph glyph, const std::string& value,
                       const unsigned accent, const bool active = false, const std::string& marker = "")
{
    DrawSoftPanel(Rect(pos, size), active ? 0xDD5A3F26 : COLOR_PANEL_ALT);
    Window::DrawLine(pos + DrawPoint(6, size.y - 2), pos + DrawPoint(size.x - 6, size.y - 2), 1, accent);
    DrawGlyph(pos + DrawPoint(6, 4), glyph, accent);
    SmallFont->Draw(pos + DrawPoint(size.x - 8, 6), value, FontStyle::RIGHT, COLOR_YELLOW);
    if(!marker.empty())
        SmallFont->Draw(pos + DrawPoint(size.x - 5, size.y - 12), marker, FontStyle::RIGHT, COLOR_MUTED);
}
} // namespace

void DrawDeveloperHudOverlay(const DeveloperHudViewModel& data, const Extent screenSize)
{
    const int width = static_cast<int>(screenSize.x);
    const int height = static_cast<int>(screenSize.y);
    const int centerX = width / 2;

    DrawSoftPanel(Rect(DrawPoint(18, 8), Extent(screenSize.x - 36, 34)), COLOR_PANEL);
    SmallFont->Draw(DrawPoint(30, 18), "DEV HUD", FontStyle{}, COLOR_ACCENT);
    SmallFont->Draw(DrawPoint(92, 18), data.isLiveData ? "live" : "mock", FontStyle{},
                    data.isLiveData ? COLOR_GOOD : COLOR_WARNING);
    SmallFont->Draw(DrawPoint(width - 280, 18), data.developmentExportSummary, FontStyle{}, COLOR_ACCENT);

    DrawPoint chipPos(centerX - 164, 12);
    DrawIconValueChip(chipPos, Extent(58, 24), HudGlyph::Player, PlayerChipValue(data.playerLabel), COLOR_ACCENT, true);
    DrawIconValueChip(chipPos + DrawPoint(66, 0), Extent(58, 24), HudGlyph::Soldier, std::to_string(data.totalSoldiers),
                      COLOR_WARNING);
    DrawIconValueChip(chipPos + DrawPoint(132, 0), Extent(58, 24), HudGlyph::Gold, data.resourceChips[0].value,
                      COLOR_GOOD);
    DrawIconValueChip(chipPos + DrawPoint(198, 0), Extent(58, 24), HudGlyph::Sword, data.resourceChips[1].value,
                      COLOR_GOOD);
    DrawIconValueChip(chipPos + DrawPoint(264, 0), Extent(58, 24), HudGlyph::Post, std::to_string(data.messageCount),
                      data.messageCount > 0 ? COLOR_WARNING : COLOR_GOOD, data.messageCount > 0);

    const DrawPoint railPos(18, 58);
    DrawSoftPanel(Rect(railPos, Extent(48, 210)), COLOR_PANEL);
    const std::array<HudGlyph, 6> railGlyphs = {
      {HudGlyph::Build, HudGlyph::Road, HudGlyph::Military, HudGlyph::Economy, HudGlyph::Post, HudGlyph::Map}};
    for(unsigned i = 0; i < railGlyphs.size(); ++i)
    {
        const DrawPoint buttonPos = railPos + DrawPoint(8, 12 + static_cast<int>(i) * 32);
        DrawIconValueChip(buttonPos, Extent(32, 22), railGlyphs[i], "", i == 2 ? COLOR_WARNING : COLOR_ACCENT);
    }

    const DrawPoint mapPos(width - 190, 56);
    DrawSoftPanel(Rect(mapPos, Extent(164, 108)), COLOR_PANEL);
    SmallFont->Draw(mapPos + DrawPoint(14, 9), "plh", FontStyle{}, COLOR_MUTED);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(14, 26), Extent(136, 58)), 0xFF244966);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(30, 36), Extent(76, 30)), 0xFF3E6833);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(84, 54), Extent(48, 20)), 0xFF987A38);
    DrawIconValueChip(mapPos + DrawPoint(14, 86), Extent(90, 18), HudGlyph::Map,
                      data.mapSummary.substr(4, data.mapSummary.find(" |") - 4), COLOR_ACCENT);

    const DrawPoint bottomPos(18, height - 48);
    DrawSoftPanel(Rect(bottomPos, Extent(screenSize.x - 36, 34)), COLOR_PANEL);

    DrawPoint resourcePos(centerX - 198, height - 44);
    for(unsigned i = 0; i < data.resourceChips.size(); ++i)
    {
        const DeveloperHudResourceChip& chip = data.resourceChips[i];
        DrawIconValueChip(resourcePos + DrawPoint(static_cast<int>(i) * 66, 0), Extent(58, 24),
                          ResourceGlyph(chip.icon), chip.value, i == 2 ? COLOR_WARNING : COLOR_GOOD);
    }

    Window::DrawRectangle(Rect(DrawPoint(width - 86, height - 78), Extent(60, 18)), COLOR_DIM);
    SmallFont->Draw(DrawPoint(width - 56, height - 73), "no cmd", FontStyle::CENTER, COLOR_MUTED);
}
