// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "Loader.h"
#include "RttrConfig.h"
#include "Settings.h"
#include "TextureFiltering.h"
#include "controls/ctrlComboBox.h"
#include "controls/ctrlGroup.h"
#include "controls/ctrlText.h"
#include "desktops/dskOptions.h"
#include "files.h"
#include "uiHelper/uiHelpers.hpp"
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>
#include <mygettext/mygettext.h>

namespace {
struct TextureFilteringGuard
{
    TextureFilteringGuard() : value(SETTINGS.video.textureFiltering) {}
    ~TextureFilteringGuard() { SETTINGS.video.textureFiltering = value; }

    TextureFiltering value;
};
} // namespace

BOOST_FIXTURE_TEST_CASE(GraphicsOptionsExposeTextureFiltering, uiHelper::Fixture)
{
    TextureFilteringGuard guard;
    SETTINGS.video.textureFiltering = GetDefaultTextureFiltering();

    LOADER.Load(RTTRCONFIG.ExpandPath(s25::folders::assetsBase) / "languages.ini", nullptr);
    const dskOptions options;

    bool labelFound = false;
    const ctrlComboBox* filteringCombo = nullptr;
    for(const ctrlGroup* group : options.GetCtrls<ctrlGroup>())
    {
        for(const ctrlText* text : group->GetCtrls<ctrlText>())
            labelFound |= text->GetText() == _("Texture filtering");

        for(const ctrlComboBox* combo : group->GetCtrls<ctrlComboBox>())
        {
            if(combo->GetNumItems() == 2u && combo->GetText(0) == _("Pixel / sharp")
               && combo->GetText(1) == _("Smooth"))
            {
                filteringCombo = combo;
            }
        }
    }

    BOOST_TEST(labelFound);
    BOOST_TEST_REQUIRE(filteringCombo);
    BOOST_TEST_REQUIRE(filteringCombo->GetSelection());
    BOOST_TEST(filteringCombo->GetSelection().value() == static_cast<unsigned>(TextureFiltering::Pixel));
    BOOST_TEST_REQUIRE(filteringCombo->GetSelectedText());
    BOOST_TEST(filteringCombo->GetSelectedText().value() == _("Pixel / sharp"));
}
