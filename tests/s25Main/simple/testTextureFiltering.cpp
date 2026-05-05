// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "GlobalGameSettings.h"
#include "TextureFiltering.h"
#include "addons/const_addons.h"
#include "helpers/MaxEnumValue.h"
#include "ogl/TextureFilter.h"
#include "rttr/test/stubFunction.hpp"
#include <s25util/warningSuppression.h>
#include <glad/glad.h>
#include <boost/test/unit_test.hpp>
#include <vector>

namespace {
struct TexParameterCall
{
    GLenum target;
    GLenum pname;
    GLint param;
};
std::vector<TexParameterCall> texParameterCalls;
} // namespace

namespace rttrOglMockTextureFiltering {
RTTR_IGNORE_DIAGNOSTIC("-Wmissing-declarations")

void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    texParameterCalls.push_back({target, pname, param});
}

RTTR_POP_DIAGNOSTIC
} // namespace rttrOglMockTextureFiltering

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

BOOST_AUTO_TEST_CASE(TextureFilteringConfigParseIsDeterministic)
{
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue(0)) == static_cast<int>(TextureFiltering::Pixel));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("0")) == static_cast<int>(TextureFiltering::Pixel));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("pixel"))
               == static_cast<int>(TextureFiltering::Pixel));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("nearest"))
               == static_cast<int>(TextureFiltering::Pixel));

    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue(1)) == static_cast<int>(TextureFiltering::Smooth));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("1")) == static_cast<int>(TextureFiltering::Smooth));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue(" 1 ")) == static_cast<int>(TextureFiltering::Smooth));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("smooth"))
               == static_cast<int>(TextureFiltering::Smooth));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("linear"))
               == static_cast<int>(TextureFiltering::Smooth));

    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue(-1))
               == static_cast<int>(GetDefaultTextureFiltering()));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue(99))
               == static_cast<int>(GetDefaultTextureFiltering()));
    BOOST_TEST(static_cast<int>(ParseTextureFilteringConfigValue("invalid"))
               == static_cast<int>(GetDefaultTextureFiltering()));
}

BOOST_AUTO_TEST_CASE(ApplyTextureFilterSetsMinAndMagFilters)
{
    RTTR_STUB_FUNCTION(glTexParameteri, rttrOglMockTextureFiltering::glTexParameteri);

    texParameterCalls.clear();
    ApplyTextureFilter(TextureFiltering::Pixel);
    BOOST_TEST_REQUIRE(texParameterCalls.size() == 2u);
    BOOST_TEST(texParameterCalls[0].target == GL_TEXTURE_2D);
    BOOST_TEST(texParameterCalls[0].pname == GL_TEXTURE_MIN_FILTER);
    BOOST_TEST(texParameterCalls[0].param == GL_NEAREST);
    BOOST_TEST(texParameterCalls[1].target == GL_TEXTURE_2D);
    BOOST_TEST(texParameterCalls[1].pname == GL_TEXTURE_MAG_FILTER);
    BOOST_TEST(texParameterCalls[1].param == GL_NEAREST);

    texParameterCalls.clear();
    ApplyTextureFilter(TextureFiltering::Smooth);
    BOOST_TEST_REQUIRE(texParameterCalls.size() == 2u);
    BOOST_TEST(texParameterCalls[0].param == GL_LINEAR);
    BOOST_TEST(texParameterCalls[1].param == GL_LINEAR);
}

BOOST_AUTO_TEST_SUITE_END()
