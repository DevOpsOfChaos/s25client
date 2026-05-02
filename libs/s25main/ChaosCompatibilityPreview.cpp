// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityPreview.h"

namespace chaos {

CompatibilityPreview BuildCompatibilityPreview(const ContentCompatibilityResult& result)
{
    switch(result.metadataResult.status)
    {
        case MetadataReadStatus::Missing:
            return {CompatibilityPreviewStatus::Neutral, "RTTR-compatible / no Chaos metadata"};
        case MetadataReadStatus::Invalid: return {CompatibilityPreviewStatus::Invalid, "Invalid Chaos metadata"};
        case MetadataReadStatus::Valid:
            if(result.decision.allowed)
                return {CompatibilityPreviewStatus::Compatible, "Chaos Edition compatible"};
            if(result.decision.reasonKey == std::string("chaos.compatibility.rules_profile_mismatch"))
                return {CompatibilityPreviewStatus::Incompatible, "Incompatible: requires a different rules profile"};
            if(!result.decision.missingRequiredFeatures.empty())
                return {CompatibilityPreviewStatus::Incompatible,
                        "Incompatible: requires unsupported Chaos compatibility features"};
            return {CompatibilityPreviewStatus::Incompatible, "Incompatible: Chaos requirements are not satisfied"};
    }
    return {CompatibilityPreviewStatus::Incompatible, "Incompatible: Chaos requirements are not satisfied"};
}

} // namespace chaos
