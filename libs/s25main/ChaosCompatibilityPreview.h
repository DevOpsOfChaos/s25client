// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ChaosCompatibilityMetadata.h"

#include <string>

namespace chaos {

enum class CompatibilityPreviewStatus
{
    Neutral,
    Compatible,
    Invalid,
    Incompatible
};

struct CompatibilityPreview
{
    CompatibilityPreviewStatus status;
    std::string text;
};

CompatibilityPreview BuildCompatibilityPreview(const ContentCompatibilityResult& result);

} // namespace chaos
