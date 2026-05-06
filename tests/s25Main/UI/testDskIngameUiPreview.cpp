// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "controls/ctrlButton.h"
#include "controls/ctrlText.h"
#include "controls/ctrlTextButton.h"
#include "desktops/dskIngameUiPreview.h"
#include "uiHelper/uiHelpers.hpp"
#include <boost/test/unit_test.hpp>
#include <algorithm>
#include <string>

namespace {
bool HasText(const dskIngameUiPreview& preview, const std::string& expected)
{
    const auto texts = preview.GetCtrls<ctrlText>();
    return std::any_of(texts.begin(), texts.end(), [&expected](const ctrlText* text) {
        return text->GetText().find(expected) != std::string::npos;
    });
}

bool HasTextButton(const dskIngameUiPreview& preview, const std::string& expected)
{
    const auto buttons = preview.GetCtrls<ctrlButton>();
    return std::any_of(buttons.begin(), buttons.end(), [&expected](const ctrlButton* button) {
        const auto* textButton = dynamic_cast<const ctrlTextButton*>(button);
        return textButton && textButton->GetText() == expected;
    });
}
} // namespace

BOOST_FIXTURE_TEST_CASE(IngameUiPreviewWorkbenchConstructsDeveloperOnlySurface, uiHelper::Fixture)
{
    dskIngameUiPreview preview;
    preview.Draw();

    BOOST_TEST(HasText(preview, "Developer Preview: Ingame UI Workbench"));
    BOOST_TEST(HasText(preview, "Dev-only visual review surface"));
    BOOST_TEST(HasText(preview, "Preview size:"));
    BOOST_TEST(HasText(preview, "GUI scale:"));
    BOOST_TEST(HasText(preview, "Texture filtering:"));
    BOOST_TEST(HasText(preview, "Representative resources"));
    BOOST_TEST(HasText(preview, "Simulated minimap overlay"));

    BOOST_TEST(HasTextButton(preview, "Map"));
    BOOST_TEST(HasTextButton(preview, "Main"));
    BOOST_TEST(HasTextButton(preview, "Build"));
    BOOST_TEST(HasTextButton(preview, "Post"));
    BOOST_TEST(HasTextButton(preview, "Back"));
}
