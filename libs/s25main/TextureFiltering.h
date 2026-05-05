// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <cstdint>
#include <string>

enum class TextureFiltering : uint8_t
{
    Pixel,
    Smooth
};

constexpr auto maxEnumValue(TextureFiltering)
{
    return TextureFiltering::Smooth;
}

constexpr TextureFiltering GetDefaultTextureFiltering()
{
    return TextureFiltering::Pixel;
}

TextureFiltering ParseTextureFilteringConfigValue(int value);
TextureFiltering ParseTextureFilteringConfigValue(const std::string& value);
