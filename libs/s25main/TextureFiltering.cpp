// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "TextureFiltering.h"
#include "helpers/MaxEnumValue.h"
#include "helpers/strUtils.h"
#include <algorithm>
#include <cctype>

namespace {
std::string normalize(std::string value)
{
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}
} // namespace

TextureFiltering ParseTextureFilteringConfigValue(const int value)
{
    if(value >= 0 && static_cast<unsigned>(value) <= helpers::MaxEnumValue_v<TextureFiltering>)
        return static_cast<TextureFiltering>(value);
    return GetDefaultTextureFiltering();
}

TextureFiltering ParseTextureFilteringConfigValue(const std::string& value)
{
    int numericValue;
    if(helpers::tryFromString(value, numericValue))
        return ParseTextureFilteringConfigValue(numericValue);

    const std::string normalized = normalize(value);
    if(helpers::tryFromString(normalized, numericValue))
        return ParseTextureFilteringConfigValue(numericValue);

    if(normalized == "pixel" || normalized == "sharp" || normalized == "nearest")
        return TextureFiltering::Pixel;
    if(normalized == "smooth" || normalized == "linear")
        return TextureFiltering::Smooth;
    return GetDefaultTextureFiltering();
}
