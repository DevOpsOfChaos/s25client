// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "desktops/Desktop.h"

/// Developer-only visual review surface for the ingame UI shell.
class dskIngameUiPreview : public Desktop
{
public:
    dskIngameUiPreview();

    void Resize(const Extent& newSize) override;
    void Msg_ButtonClick(unsigned ctrl_id) override;
    void Msg_PaintBefore() override;
    bool Msg_KeyDown(const KeyEvent& ke) override;

private:
    void LayoutControls();
    void UpdateDiagnostics();
    void DrawPreviewBackground() const;
    void DrawIngameShell() const;
};
