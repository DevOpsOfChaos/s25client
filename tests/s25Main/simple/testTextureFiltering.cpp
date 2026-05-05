// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GlobalGameSettings.h"
#include "TextureFiltering.h"
#include "addons/const_addons.h"
#include "helpers/MaxEnumValue.h"
#include "ogl/TextureFilter.h"
#include <glad/glad.h>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TextureFilteringSuite)

BOOST_AUTO_TEST_CASE(DefaultTextureFilteringIsConservative)
{
    BOOST_TEST(static_cast<int>(GetDefaultTextureFiltering()) == static_cast<int>(TextureFiltering::Pixel));
}

BOOST_AUTO_TEST_CASE(TextureFilteringDoesNotChangeGlobalGameSettings)
{
    GlobalGameSettings gameSettings(RulesProfile::Chaos);
    BOOST_TEST(gameSettings.isEnabled(AddonId::TOOL_ORDERING));
    BOOST_TEST(gameSettings.isEnabled(AddonId::AUTOFLAGS));
}

BOOST_AUTO_TEST_CASE(TextureFilteringValuesMapToExpectedGLFilters)
{
    BOOST_TEST(helpers::MaxEnumValue_v<TextureFiltering> == 1u);
    BOOST_TEST(GetGLTextureFilter(TextureFiltering::Pixel) == GL_NEAREST);
    BOOST_TEST(GetGLTextureFilter(TextureFiltering::Smooth) == GL_LINEAR);
}

BOOST_AUTO_TEST_SUITE_END()
