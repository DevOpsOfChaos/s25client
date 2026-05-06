// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "controls/ctrlButton.h"
#include "controls/ctrlText.h"
#include "controls/ctrlTextButton.h"
#include "desktops/dskIngameUiPreview.h"
#include "driver/KeyEvent.h"
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

const ctrlTextButton* FindTextButton(const dskIngameUiPreview& preview, const std::string& expected)
{
    const auto buttons = preview.GetCtrls<ctrlButton>();
    const auto buttonIt = std::find_if(buttons.begin(), buttons.end(), [&expected](const ctrlButton* button) {
        const auto* textButton = dynamic_cast<const ctrlTextButton*>(button);
        return textButton && textButton->GetText() == expected;
    });
    return buttonIt != buttons.end() ? dynamic_cast<const ctrlTextButton*>(*buttonIt) : nullptr;
}
} // namespace

BOOST_FIXTURE_TEST_CASE(IngameUiPreviewWorkbenchConstructsDeveloperOnlyTogglePanelSurface, uiHelper::Fixture)
{
    dskIngameUiPreview preview;
    preview.Draw();

    BOOST_TEST(HasText(preview, "Developer Preview: Ingame UI Workbench"));
    BOOST_TEST(HasText(preview, "Static mock data"));
    BOOST_TEST(HasText(preview, "Not product UI"));
    BOOST_TEST(HasText(preview, "No gameplay logic"));
    BOOST_TEST(HasText(preview, "no map/simulation/network/save"));
    BOOST_TEST(HasText(preview, "Preview size:"));
    BOOST_TEST(HasText(preview, "GUI scale:"));
    BOOST_TEST(HasText(preview, "Texture filtering:"));
    BOOST_TEST(HasText(preview, "Active preview state: Classic-inspired Compact HUD"));
    BOOST_TEST(HasText(preview, "Keys 1-6 select reduced review states"));
    BOOST_TEST(HasText(preview, "Modernized classic toggle-panel UI"));
    BOOST_TEST(HasText(preview, "No legacy main/submenu wall"));
    BOOST_TEST(HasText(preview, "HUD data source: Static mock data"));
    BOOST_TEST(HasText(preview, "Compact chips: S 184 | Au 74 | Sw 38 | Msg 3"));
    BOOST_TEST(HasText(preview, "Resources panel: Gold 74 | Swords 38 | Food 214 | Coins 1256"));
    BOOST_TEST(HasText(preview, "Message lane: 3 unread post messages"));
    BOOST_TEST(HasText(preview, "Selected 44,52 | headquarters mock"));
    BOOST_TEST(HasText(preview, "Map 96x96 | read-only"));
    BOOST_TEST(HasText(preview, "HUD export contract: 8 groups / 27 fields"));
    BOOST_TEST(HasText(preview, "represented: resources, military, messages, selection, map, commands, economy"));
    BOOST_TEST(HasText(preview, "Availability: resources.gold=mock"));
    BOOST_TEST(HasText(preview, "minimap.thumbnail=placeholder"));
    BOOST_TEST(HasText(preview, "commands.dispatch=placeholder"));

    BOOST_TEST(HasTextButton(preview, "1 Compact HUD"));
    BOOST_TEST(HasTextButton(preview, "2 Build/Roads"));
    BOOST_TEST(HasTextButton(preview, "3 Mil/Eco"));
    BOOST_TEST(HasTextButton(preview, "4 Msg/Map"));
    BOOST_TEST(HasTextButton(preview, "5 Selection"));
    BOOST_TEST(HasTextButton(preview, "6 Small"));
    BOOST_TEST(HasTextButton(preview, "Build"));
    BOOST_TEST(HasTextButton(preview, "Roads"));
    BOOST_TEST(HasTextButton(preview, "Military"));
    BOOST_TEST(HasTextButton(preview, "Economy"));
    BOOST_TEST(HasTextButton(preview, "Messages"));
    BOOST_TEST(HasTextButton(preview, "Map"));
    BOOST_TEST(HasTextButton(preview, "Selection"));
    BOOST_TEST(HasTextButton(preview, "Back"));

    BOOST_TEST(!HasTextButton(preview, "E Modern HUD"));
    BOOST_TEST(!HasTextButton(preview, "2 Legacy menu"));
    BOOST_TEST(!HasTextButton(preview, "3 Edge drawers"));
}

BOOST_FIXTURE_TEST_CASE(IngameUiPreviewWorkbenchSwitchesReducedTogglePanelStates, uiHelper::Fixture)
{
    dskIngameUiPreview preview;

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('2')));
    BOOST_TEST(HasText(preview, "Active preview state: Toggle Panels: Build/Roads"));
    BOOST_TEST(HasText(preview, "Build and Roads buttons act as on/off toggles"));
    BOOST_TEST(HasText(preview, "House category and road/flag tools"));
    BOOST_TEST(HasText(preview, "Commands group is visible but dispatch stays disabled"));
    BOOST_TEST(HasText(preview, "No terrain, ownership, pathfinding"));
    BOOST_TEST(HasTextButton(preview, "Road"));
    BOOST_TEST(HasTextButton(preview, "Flag"));
    const ctrlTextButton* disabledButton = FindTextButton(preview, "Disabled");
    BOOST_TEST_REQUIRE(disabledButton);
    BOOST_TEST(!disabledButton->GetEnabled());

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('3')));
    BOOST_TEST(HasText(preview, "Active preview state: Toggle Panels: Military/Economy"));
    BOOST_TEST(HasText(preview, "Military panel uses military group"));
    BOOST_TEST(HasText(preview, "Economy panel uses resources and economy groups"));
    BOOST_TEST(HasText(preview, "No barracks, production, combat"));
    BOOST_TEST(HasTextButton(preview, "Readiness"));
    BOOST_TEST(HasTextButton(preview, "Resources"));

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('4')));
    BOOST_TEST(HasText(preview, "Active preview state: Messages + Minimap"));
    BOOST_TEST(HasText(preview, "Messages panel uses messages group"));
    BOOST_TEST(HasText(preview, "Map panel uses map group"));
    BOOST_TEST(HasText(preview, "Zoom + / Zoom - / collapse / expand"));
    BOOST_TEST(HasText(preview, "No real map data"));
    BOOST_TEST(HasTextButton(preview, "Mute"));
    BOOST_TEST(HasTextButton(preview, "Zoom +"));

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('5')));
    BOOST_TEST(HasText(preview, "Active preview state: Selected Object / Context Panel"));
    BOOST_TEST(HasText(preview, "Selection button opens a compact context panel"));
    BOOST_TEST(HasText(preview, "Selection group feeds selected object summary"));
    BOOST_TEST(HasText(preview, "command dispatch remains disabled"));
    BOOST_TEST(HasText(preview, "No object command"));
    BOOST_TEST(HasTextButton(preview, "Inspect"));
    BOOST_TEST(HasTextButton(preview, "Center"));
    const ctrlTextButton* commandButton = FindTextButton(preview, "Command");
    BOOST_TEST_REQUIRE(commandButton);
    BOOST_TEST(!commandButton->GetEnabled());

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('6')));
    BOOST_TEST(HasText(preview, "Active preview state: Small-screen Stress"));
    BOOST_TEST(HasText(preview, "1280x720 style safe-area"));
    BOOST_TEST(HasText(preview, "No screenshot-comparison"));
    BOOST_TEST(HasText(preview, "fix it before product UI work"));

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent('1')));
    BOOST_TEST(HasText(preview, "Active preview state: Classic-inspired Compact HUD"));
    BOOST_TEST(HasText(preview, "narrow top bar, narrow bottom bar"));
    BOOST_TEST(HasText(preview, "Toggle buttons: Build, Roads, Military, Economy, Messages, Map, Selection"));
}

BOOST_FIXTURE_TEST_CASE(IngameUiPreviewWorkbenchCyclesStatesWithTab, uiHelper::Fixture)
{
    dskIngameUiPreview preview;

    BOOST_TEST(preview.Msg_KeyDown(KeyEvent(KeyType::Tab)));
    BOOST_TEST(HasText(preview, "Active preview state: Toggle Panels: Build/Roads"));

    KeyEvent reverseTab(KeyType::Tab);
    reverseTab.shift = true;
    BOOST_TEST(preview.Msg_KeyDown(reverseTab));
    BOOST_TEST(HasText(preview, "Active preview state: Classic-inspired Compact HUD"));
}
