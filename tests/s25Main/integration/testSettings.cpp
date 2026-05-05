// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RttrConfig.h"
#include "Settings.h"
#include "TextureFiltering.h"
#include "files.h"
#include "libsiedler2/ArchivItem_Ini.h"
#include "libsiedler2/libsiedler2.h"
#include "rttr/test/ConfigOverride.hpp"
#include "rttr/test/TmpFolder.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>
#include <map>
#include <string>

namespace {
struct SettingsGuard
{
    SettingsGuard()
        : textureFiltering(SETTINGS.video.textureFiltering), addonConfiguration(SETTINGS.addons.configuration)
    {}

    ~SettingsGuard()
    {
        SETTINGS.video.textureFiltering = textureFiltering;
        SETTINGS.addons.configuration = addonConfiguration;
    }

    TextureFiltering textureFiltering;
    std::map<unsigned, unsigned> addonConfiguration;
};

void writeTextureFilteringConfigValue(const std::string& value)
{
    libsiedler2::Archiv settings;
    const auto settingsPath = RTTRCONFIG.ExpandPath(s25::resources::config);
    BOOST_TEST_REQUIRE(libsiedler2::Load(settingsPath, settings) == 0);
    auto* iniVideo = dynamic_cast<libsiedler2::ArchivItem_Ini*>(settings.find("video"));
    BOOST_TEST_REQUIRE(iniVideo);
    iniVideo->setValue("texture_filtering", value);
    BOOST_TEST_REQUIRE(libsiedler2::Write(settingsPath, settings) == 0);
}

void checkTextureFilteringRoundTrip(TextureFiltering filtering)
{
    rttr::test::TmpFolder tmp;
    rttr::test::ConfigOverride overrideUserData("USERDATA", tmp.get());
    SettingsGuard guard;
    const std::map<unsigned, unsigned> addonConfiguration = {{7u, 3u}, {19u, 1u}};

    SETTINGS.video.textureFiltering = filtering;
    SETTINGS.addons.configuration = addonConfiguration;
    SETTINGS.Save();

    SETTINGS.video.textureFiltering =
      filtering == TextureFiltering::Pixel ? TextureFiltering::Smooth : TextureFiltering::Pixel;
    SETTINGS.addons.configuration.clear();
    SETTINGS.Load();

    BOOST_TEST(static_cast<int>(SETTINGS.video.textureFiltering) == static_cast<int>(filtering));
    BOOST_TEST((SETTINGS.addons.configuration == addonConfiguration));
    BOOST_TEST(boost::filesystem::exists(RTTRCONFIG.ExpandPath(s25::resources::config)));
}
} // namespace

BOOST_AUTO_TEST_SUITE(SettingsSuite)

BOOST_AUTO_TEST_CASE(CheckPort)
{
    BOOST_TEST_REQUIRE(!validate::checkPort("-1"));
    BOOST_TEST_REQUIRE(!validate::checkPort("0"));
    BOOST_TEST_REQUIRE(!validate::checkPort("65536"));
    BOOST_TEST_REQUIRE(!validate::checkPort("-1"));
    BOOST_TEST_REQUIRE(!validate::checkPort("1-6"));
    BOOST_TEST_REQUIRE(!validate::checkPort("1.1"));
    boost::optional<uint16_t> port = validate::checkPort("1");
    BOOST_TEST_REQUIRE(port);
    BOOST_TEST_REQUIRE(*port == 1u);
    port = validate::checkPort("100");
    BOOST_TEST_REQUIRE(port);
    BOOST_TEST_REQUIRE(*port == 100u);
    port = validate::checkPort("65535");
    BOOST_TEST_REQUIRE(port);
    BOOST_TEST_REQUIRE(*port == 65535u);
}

BOOST_AUTO_TEST_CASE(VideoTextureFilteringPixelPersistsInLocalSettings)
{
    checkTextureFilteringRoundTrip(TextureFiltering::Pixel);
}

BOOST_AUTO_TEST_CASE(VideoTextureFilteringSmoothPersistsInLocalSettings)
{
    checkTextureFilteringRoundTrip(TextureFiltering::Smooth);
}

BOOST_AUTO_TEST_CASE(VideoTextureFilteringInvalidConfigFallsBackToDefault)
{
    rttr::test::TmpFolder tmp;
    rttr::test::ConfigOverride overrideUserData("USERDATA", tmp.get());
    SettingsGuard guard;
    const std::map<unsigned, unsigned> addonConfiguration = {{7u, 3u}, {19u, 1u}};

    SETTINGS.video.textureFiltering = TextureFiltering::Smooth;
    SETTINGS.addons.configuration = addonConfiguration;
    SETTINGS.Save();

    writeTextureFilteringConfigValue("invalid");
    SETTINGS.Load();

    BOOST_TEST(static_cast<int>(SETTINGS.video.textureFiltering) == static_cast<int>(GetDefaultTextureFiltering()));
    BOOST_TEST((SETTINGS.addons.configuration == addonConfiguration));
}

BOOST_AUTO_TEST_SUITE_END()
