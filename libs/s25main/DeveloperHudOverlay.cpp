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

namespace {
constexpr unsigned COLOR_PANEL = 0xDD111A22;
constexpr unsigned COLOR_PANEL_ALT = 0xCC182833;
constexpr unsigned COLOR_ACCENT = 0xFF68C7D8;
constexpr unsigned COLOR_GOOD = 0xFF83C568;
constexpr unsigned COLOR_WARNING = 0xFFD67B4A;
constexpr unsigned COLOR_DIM = 0xCC0B1116;

void DrawSoftPanel(const Rect& rect, const unsigned color)
{
    const DrawPoint pos = rect.getOrigin();
    const Extent size = rect.getSize();
    Window::DrawRectangle(Rect(pos + DrawPoint(4, 0), Extent(size.x - 8, size.y)), color);
    Window::DrawRectangle(Rect(pos + DrawPoint(0, 4), Extent(size.x, size.y - 8)), color);
}

void DrawChip(const DrawPoint pos, const Extent size, const std::string& text, const unsigned accent)
{
    DrawSoftPanel(Rect(pos, size), COLOR_PANEL_ALT);
    Window::DrawLine(pos + DrawPoint(6, size.y - 2), pos + DrawPoint(size.x - 6, size.y - 2), 1, accent);
    SmallFont->Draw(pos + DrawPoint(10, 6), text, FontStyle{}, COLOR_YELLOW);
}
} // namespace

void DrawDeveloperHudOverlay(const DeveloperHudViewModel& data, const Extent screenSize)
{
    const int width = static_cast<int>(screenSize.x);
    const int height = static_cast<int>(screenSize.y);
    const int centerX = width / 2;

    DrawSoftPanel(Rect(DrawPoint(18, 8), Extent(screenSize.x - 36, 34)), COLOR_PANEL);
    SmallFont->Draw(DrawPoint(30, 18), "Developer Preview / HUD Data Export", FontStyle{}, COLOR_ACCENT);
    SmallFont->Draw(DrawPoint(260, 18), data.sourceLabel, FontStyle{}, data.isLiveData ? COLOR_GOOD : COLOR_WARNING);
    SmallFont->Draw(DrawPoint(width - 280, 18), data.developmentExportSummary, FontStyle{}, COLOR_ACCENT);

    DrawPoint chipPos(centerX - 240, 12);
    for(unsigned i = 0; i < data.topBarChips.size(); ++i)
    {
        DrawChip(chipPos + DrawPoint(static_cast<int>(i) * 96, 0), Extent(88, 24), data.topBarChips[i],
                 i == 1 ? COLOR_WARNING : COLOR_GOOD);
    }

    const DrawPoint lanePos(centerX - 245, 52);
    DrawSoftPanel(Rect(lanePos, Extent(490, 34)), COLOR_PANEL);
    Window::DrawLine(lanePos + DrawPoint(12, 32), lanePos + DrawPoint(478, 32), 1, COLOR_ACCENT);
    SmallFont->Draw(lanePos + DrawPoint(16, 10), data.messageLane, FontStyle{}, COLOR_YELLOW);

    const DrawPoint railPos(18, 96);
    DrawSoftPanel(Rect(railPos, Extent(54, 220)), COLOR_PANEL);
    const std::array<const char*, 6> railLabels = {{"B", "R", "M", "E", "P", "!"}};
    for(unsigned i = 0; i < railLabels.size(); ++i)
    {
        const DrawPoint buttonPos = railPos + DrawPoint(10, 14 + static_cast<int>(i) * 34);
        DrawChip(buttonPos, Extent(34, 23), railLabels[i], i == 2 ? COLOR_WARNING : COLOR_ACCENT);
    }

    const DrawPoint mapPos(width - 210, 56);
    DrawSoftPanel(Rect(mapPos, Extent(184, 124)), COLOR_PANEL);
    SmallFont->Draw(mapPos + DrawPoint(12, 10), "Minimap probe", FontStyle{}, COLOR_ACCENT);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(14, 32), Extent(156, 64)), 0xFF1F4464);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(32, 44), Extent(88, 34)), 0xFF3E6833);
    Window::DrawRectangle(Rect(mapPos + DrawPoint(92, 64), Extent(54, 22)), 0xFF977A34);
    SmallFont->Draw(mapPos + DrawPoint(14, 102), data.mapSummary, FontStyle{}, COLOR_YELLOW);

    const DrawPoint bottomPos(18, height - 48);
    DrawSoftPanel(Rect(bottomPos, Extent(screenSize.x - 36, 34)), COLOR_PANEL);
    SmallFont->Draw(bottomPos + DrawPoint(14, 10), data.militarySummary, FontStyle{}, COLOR_YELLOW);
    SmallFont->Draw(DrawPoint(centerX - 70, height - 38), data.selectedSummary, FontStyle{}, COLOR_YELLOW);

    DrawPoint resourcePos(width - 590, height - 44);
    for(unsigned i = 0; i < data.resourceChips.size(); ++i)
    {
        const DeveloperHudResourceChip& chip = data.resourceChips[i];
        DrawChip(resourcePos + DrawPoint(static_cast<int>(i) * 96, 0), Extent(88, 24), chip.icon + " " + chip.value,
                 i == 2 ? COLOR_WARNING : COLOR_GOOD);
    }

    Window::DrawRectangle(Rect(DrawPoint(width - 154, height - 86), Extent(128, 24)), COLOR_DIM);
    SmallFont->Draw(DrawPoint(width - 90, height - 79), "read-only / no commands", FontStyle::CENTER, COLOR_ACCENT);
}
