// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "DeveloperHudDataProvider.h"
#include "desktops/Desktop.h"
#include <array>
#include <memory>

/// Developer-only visual review surface for the ingame UI shell.
class dskIngameUiPreview : public Desktop
{
public:
    enum class PreviewState
    {
        ClassicCompactHud,
        ToggleBuildRoads,
        ToggleMilitaryEconomy,
        MessagesMinimap,
        SelectionContext,
        SmallScreenStress,
        Count
    };

    dskIngameUiPreview();

    void Resize(const Extent& newSize) override;
    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_PaintBefore() override;
    bool Msg_KeyDown(const KeyEvent& ke) override;

private:
    void SetPreviewState(PreviewState state);
    void CyclePreviewState(int direction);
    void LayoutControls();
    void UpdateDiagnostics();
    void UpdatePreviewStateControls();
    void DrawPreviewBackground() const;
    void DrawIngameShell() const;
    void DrawPreviewState() const;
    void DrawClassicPanel(DrawPoint pos, Extent size, const std::string& title) const;
    void DrawChip(DrawPoint pos, Extent size, const std::string& label, bool active = false) const;
    void DrawMiniMap(DrawPoint pos, Extent size) const;
    void DrawActionGrid(DrawPoint pos, const std::array<const char*, 8>& labels, unsigned activeIndex,
                        unsigned disabledIndex) const;
    void RefreshHudData();

    PreviewState previewState_;
    std::unique_ptr<DeveloperHudDataProvider> hudDataProvider_;
    DeveloperHudViewModel hudData_;
};
