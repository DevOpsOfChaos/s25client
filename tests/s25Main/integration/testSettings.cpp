// Copyright (C) 2005 - 2021 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "RttrConfig.h"
#include "Settings.h"
#include "TextureFiltering.h"
#include "files.h"
#include "rttr/test/ConfigOverride.hpp"
#include "rttr/test/TmpFolder.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/optional/optional_io.hpp>
#include <boost/test/unit_test.hpp>

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

BOOST_AUTO_TEST_CASE(VideoTextureFilteringPersistsInLocalSettings)
{
    rttr::test::TmpFolder tmp;
    rttr::test::ConfigOverride overrideUserData("USERDATA", tmp.get());
    const TextureFiltering oldTextureFiltering = SETTINGS.video.textureFiltering;
    const auto oldAddonConfiguration = SETTINGS.addons.configuration;

    SETTINGS.video.textureFiltering = TextureFiltering::Smooth;
    SETTINGS.Save();

    SETTINGS.video.textureFiltering = TextureFiltering::Pixel;
    SETTINGS.Load();

    BOOST_TEST(static_cast<int>(SETTINGS.video.textureFiltering) == static_cast<int>(TextureFiltering::Smooth));
    BOOST_TEST(SETTINGS.addons.configuration.size() == oldAddonConfiguration.size());
    BOOST_TEST(boost::filesystem::exists(RTTRCONFIG.ExpandPath(s25::resources::config)));

    SETTINGS.video.textureFiltering = oldTextureFiltering;
    SETTINGS.addons.configuration = oldAddonConfiguration;
}

BOOST_AUTO_TEST_SUITE_END()
