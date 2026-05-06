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
#include "desktops/dskTest.h"
#include "driver/KeyEvent.h"
#include "drivers/VideoDriverWrapper.h"
#include "ogl/FontStyle.h"
#include "ogl/glFont.h"
#include "s25util/colors.h"
#include <algorithm>
#include <array>
#include <string>

namespace {
enum
{
    ID_txtTitle = 1,
    ID_txtNotice,
    ID_txtSize,
    ID_txtGuiScale,
    ID_txtFiltering,
    ID_txtStatusHeader,
    ID_txtResourcesHeader,
    ID_txtMinimapHeader,
    ID_txtMapHint,
    ID_txtOverlayHint,
    ID_txtUnreadPost,
    ID_txtBuildingStatus,
    ID_btMap,
    ID_btMain,
    ID_btBuildAid,
    ID_btPost,
    ID_btRoad,
    ID_btHouse,
    ID_btFlag,
    ID_btBack
};

constexpr unsigned COLOR_PANEL = 0xD0182418;
constexpr unsigned COLOR_PANEL_ALT = 0xCC243020;
constexpr unsigned COLOR_MAP_A = 0xFF335332;
constexpr unsigned COLOR_MAP_B = 0xFF406239;
constexpr unsigned COLOR_MAP_GRID = 0x804F7748;
constexpr unsigned COLOR_MINIMAP_WATER = 0xFF1F4464;
constexpr unsigned COLOR_MINIMAP_LAND = 0xFF3E6833;
constexpr unsigned COLOR_MINIMAP_FIELD = 0xFF977A34;
constexpr unsigned COLOR_PREVIEW_LINE = 0xA0E7D89B;

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

dskIngameUiPreview::dskIngameUiPreview() : Desktop(nullptr)
{
    SetScale(false);
    SetFpsDisplay(false);

    AddText(ID_txtTitle, DrawPoint(0, 0), "Developer Preview: Ingame UI Workbench", COLOR_ORANGE, FontStyle::CENTER,
            LargeFont);
    AddText(ID_txtNotice, DrawPoint(0, 0),
            "Dev-only visual review surface - no map, simulation, save, network, or replay", COLOR_YELLOW,
            FontStyle::CENTER, SmallFont);
    AddText(ID_txtSize, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtGuiScale, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);
    AddText(ID_txtFiltering, DrawPoint(0, 0), "", COLOR_YELLOW, FontStyle{}, SmallFont);

    AddText(ID_txtStatusHeader, DrawPoint(0, 0), "Status and actions", COLOR_ORANGE, FontStyle{}, NormalFont);
    AddText(ID_txtResourcesHeader, DrawPoint(0, 0), "Representative resources", COLOR_ORANGE, FontStyle{}, NormalFont);
    AddText(ID_txtMinimapHeader, DrawPoint(0, 0), "Simulated minimap overlay", COLOR_ORANGE, FontStyle{}, NormalFont);
    AddText(ID_txtMapHint, DrawPoint(0, 0), "Fake terrain background for spacing, density, and contrast review",
            COLOR_GREY, FontStyle::CENTER, SmallFont);
    AddText(ID_txtOverlayHint, DrawPoint(0, 0), "Message count, building hints, and review labels are static samples",
            COLOR_GREY, FontStyle::CENTER, SmallFont);
    AddText(ID_txtUnreadPost, DrawPoint(0, 0), "3", COLOR_YELLOW, FontStyle::CENTER | FontStyle::VCENTER, SmallFont);
    AddText(ID_txtBuildingStatus, DrawPoint(0, 0), "HQ supplies stable | Roads: preview only | GF: simulated",
            COLOR_YELLOW, FontStyle{}, SmallFont);

    AddTextButton(ID_btMap, DrawPoint(0, 0), Extent(37, 32), TextureColor::Green1, "Map", NormalFont);
    AddTextButton(ID_btMain, DrawPoint(0, 0), Extent(50, 32), TextureColor::Green1, "Main", NormalFont);
    AddTextButton(ID_btBuildAid, DrawPoint(0, 0), Extent(64, 32), TextureColor::Green1, "Build", NormalFont);
    AddTextButton(ID_btPost, DrawPoint(0, 0), Extent(44, 32), TextureColor::Green1, "Post", NormalFont);

    AddTextButton(ID_btRoad, DrawPoint(0, 0), Extent(86, 22), TextureColor::Green2, "Road", NormalFont);
    AddTextButton(ID_btHouse, DrawPoint(0, 0), Extent(86, 22), TextureColor::Green2, "House", NormalFont);
    AddTextButton(ID_btFlag, DrawPoint(0, 0), Extent(86, 22), TextureColor::Green2, "Flag", NormalFont);
    AddTextButton(ID_btBack, DrawPoint(0, 0), Extent(120, 22), TextureColor::Red1, "Back", NormalFont);

    LayoutControls();
}

void dskIngameUiPreview::Resize(const Extent& newSize)
{
    Window::Resize(newSize);
    LayoutControls();
}

void dskIngameUiPreview::Msg_ButtonClick(const unsigned ctrl_id)
{
    if(ctrl_id == ID_btBack)
        WINDOWMANAGER.Switch(std::make_unique<dskTest>());
}

void dskIngameUiPreview::Msg_PaintBefore()
{
    Desktop::Msg_PaintBefore();
    UpdateDiagnostics();
    DrawPreviewBackground();
    DrawIngameShell();
}

bool dskIngameUiPreview::Msg_KeyDown(const KeyEvent& ke)
{
    if(ke.kt == KeyType::Escape)
    {
        WINDOWMANAGER.Switch(std::make_unique<dskTest>());
        return true;
    }
    return false;
}

void dskIngameUiPreview::LayoutControls()
{
    const Extent size = GetSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const int centerX = width / 2;
    const int bottomPanelTop = std::max(390, height - 96);
    const int rightPanelLeft = std::max(560, width - 250);
    const int leftPanelTop = 78;

    GetCtrl<ctrlText>(ID_txtTitle)->SetPos(DrawPoint(centerX, 18));
    GetCtrl<ctrlText>(ID_txtNotice)->SetPos(DrawPoint(centerX, 43));
    GetCtrl<ctrlText>(ID_txtSize)->SetPos(DrawPoint(18, 20));
    GetCtrl<ctrlText>(ID_txtGuiScale)->SetPos(DrawPoint(18, 39));
    GetCtrl<ctrlText>(ID_txtFiltering)->SetPos(DrawPoint(18, 58));

    GetCtrl<ctrlText>(ID_txtStatusHeader)->SetPos(DrawPoint(22, leftPanelTop + 14));
    GetCtrl<ctrlText>(ID_txtBuildingStatus)->SetPos(DrawPoint(22, leftPanelTop + 45));
    GetCtrl<ctrlText>(ID_txtResourcesHeader)->SetPos(DrawPoint(rightPanelLeft + 18, leftPanelTop + 14));
    GetCtrl<ctrlText>(ID_txtMinimapHeader)->SetPos(DrawPoint(rightPanelLeft + 18, leftPanelTop + 146));
    GetCtrl<ctrlText>(ID_txtMapHint)->SetPos(DrawPoint(centerX, height / 2 - 12));
    GetCtrl<ctrlText>(ID_txtOverlayHint)->SetPos(DrawPoint(centerX, bottomPanelTop - 18));

    DrawPoint actionPos(22, leftPanelTop + 75);
    GetCtrl<ctrlButton>(ID_btRoad)->SetPos(actionPos);
    actionPos.y += 28;
    GetCtrl<ctrlButton>(ID_btHouse)->SetPos(actionPos);
    actionPos.y += 28;
    GetCtrl<ctrlButton>(ID_btFlag)->SetPos(actionPos);

    const int barWidth = 37 + 50 + 64 + 44;
    DrawPoint barPos(centerX - barWidth / 2, bottomPanelTop + 36);
    GetCtrl<ctrlButton>(ID_btMap)->SetPos(barPos);
    barPos.x += 37;
    GetCtrl<ctrlButton>(ID_btMain)->SetPos(barPos);
    barPos.x += 50;
    GetCtrl<ctrlButton>(ID_btBuildAid)->SetPos(barPos);
    barPos.x += 64;
    GetCtrl<ctrlButton>(ID_btPost)->SetPos(barPos);
    GetCtrl<ctrlText>(ID_txtUnreadPost)->SetPos(barPos + DrawPoint(28, 24));

    GetCtrl<ctrlButton>(ID_btBack)->SetPos(DrawPoint(width - 140, height - 34));

    UpdateDiagnostics();
}

void dskIngameUiPreview::UpdateDiagnostics()
{
    GetCtrl<ctrlText>(ID_txtSize)->SetText(GetPreviewSizeLabel());
    GetCtrl<ctrlText>(ID_txtGuiScale)->SetText(GetGuiScaleLabel());
    GetCtrl<ctrlText>(ID_txtFiltering)->SetText(GetTextureFilteringLabel());
}

void dskIngameUiPreview::DrawPreviewBackground() const
{
    const Extent size = GetSize();
    DrawRectangle(Rect(DrawPoint::all(0), size), 0xFF142014);

    constexpr unsigned tileSize = 32;
    for(unsigned y = 0; y < size.y; y += tileSize)
    {
        for(unsigned x = 0; x < size.x; x += tileSize)
        {
            const bool alternate = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            DrawRectangle(Rect(DrawPoint(x, y), Extent(tileSize, tileSize)), alternate ? COLOR_MAP_A : COLOR_MAP_B);
        }
    }

    for(unsigned x = 0; x < size.x; x += 64)
        DrawLine(DrawPoint(x, 72), DrawPoint(x, size.y), 1, COLOR_MAP_GRID);
    for(unsigned y = 72; y < size.y; y += 48)
        DrawLine(DrawPoint(0, y), DrawPoint(size.x, y), 1, COLOR_MAP_GRID);
}

void dskIngameUiPreview::DrawIngameShell() const
{
    const Extent size = GetSize();
    const int width = static_cast<int>(size.x);
    const int height = static_cast<int>(size.y);
    const int bottomPanelTop = std::max(390, height - 96);
    const int rightPanelLeft = std::max(560, width - 250);

    DrawRectangle(Rect(DrawPoint(0, 0), Extent(size.x, 72)), COLOR_PANEL);
    DrawRectangle(Rect(DrawPoint(0, bottomPanelTop), Extent(size.x, height - bottomPanelTop)), COLOR_PANEL);
    DrawRectangle(Rect(DrawPoint(0, 72), Extent(134, bottomPanelTop - 72)), COLOR_PANEL_ALT);
    DrawRectangle(Rect(DrawPoint(rightPanelLeft, 72), Extent(width - rightPanelLeft, bottomPanelTop - 72)),
                  COLOR_PANEL_ALT);

    Draw3D(Rect(DrawPoint(14, 86), Extent(106, 150)), TextureColor::Green1, true);
    Draw3D(Rect(DrawPoint(rightPanelLeft + 12, 86), Extent(width - rightPanelLeft - 24, 118)), TextureColor::Green1,
           true);
    Draw3D(Rect(DrawPoint(rightPanelLeft + 12, 218), Extent(width - rightPanelLeft - 24, 160)), TextureColor::Green1,
           true);
    Draw3D(Rect(DrawPoint(width / 2 - 132, bottomPanelTop + 22), Extent(264, 55)), TextureColor::Green1, true);

    const DrawPoint resourcePos(rightPanelLeft + 24, 126);
    constexpr Extent resourceBox(34, 24);
    const std::array<unsigned, 5> resourceColors = {{0xFFCFA64A, 0xFFB4B4B4, 0xFF6A9C43, 0xFFC97554, 0xFF8A6E46}};
    const std::array<const char*, 5> resourceLabels = {{"Wood", "Stone", "Food", "Tools", "Coins"}};
    for(unsigned i = 0; i < resourceColors.size(); ++i)
    {
        const DrawPoint pos = resourcePos + DrawPoint(static_cast<int>(i) * 42, 0);
        Draw3D(Rect(pos, resourceBox), TextureColor::Grey, false);
        DrawRectangle(Rect(pos + DrawPoint(5, 5), Extent(24, 14)), resourceColors[i]);
        SmallFont->Draw(pos + DrawPoint(17, 32), resourceLabels[i], FontStyle::CENTER, COLOR_YELLOW);
    }

    const DrawPoint minimapPos(rightPanelLeft + 28, 258);
    const Extent minimapSize(std::max(110, width - rightPanelLeft - 56), 92);
    DrawRectangle(Rect(minimapPos, minimapSize), COLOR_MINIMAP_WATER);
    DrawRectangle(Rect(minimapPos + DrawPoint(12, 10), Extent(minimapSize.x - 40, 58)), COLOR_MINIMAP_LAND);
    DrawRectangle(Rect(minimapPos + DrawPoint(38, 34), Extent(54, 34)), COLOR_MINIMAP_FIELD);
    DrawLine(minimapPos + DrawPoint(0, 0), minimapPos + DrawPoint(minimapSize.x, minimapSize.y), 1, COLOR_PREVIEW_LINE);
    DrawLine(minimapPos + DrawPoint(minimapSize.x, 0), minimapPos + DrawPoint(0, minimapSize.y), 1, COLOR_PREVIEW_LINE);

    DrawLine(DrawPoint(0, 72), DrawPoint(width, 72), 2, COLOR_PREVIEW_LINE);
    DrawLine(DrawPoint(0, bottomPanelTop), DrawPoint(width, bottomPanelTop), 2, COLOR_PREVIEW_LINE);
    DrawLine(DrawPoint(134, 72), DrawPoint(134, bottomPanelTop), 2, COLOR_PREVIEW_LINE);
    DrawLine(DrawPoint(rightPanelLeft, 72), DrawPoint(rightPanelLeft, bottomPanelTop), 2, COLOR_PREVIEW_LINE);
}
