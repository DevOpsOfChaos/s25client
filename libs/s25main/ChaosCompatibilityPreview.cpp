// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityPreview.h"

namespace chaos {

CompatibilityPreview BuildCompatibilityPreview(const ContentCompatibilityResult& result)
{
    switch(result.metadataResult.status)
    {
        case MetadataReadStatus::Missing: return {CompatibilityPreviewStatus::Neutral, "Chaos metadata: not present"};
        case MetadataReadStatus::Invalid: return {CompatibilityPreviewStatus::Invalid, "Chaos metadata: invalid"};
        case MetadataReadStatus::Valid:
            if(result.decision.allowed)
            {
                std::string text = "Chaos metadata: compatible";
                if(!result.metadataResult.metadata.requiredFeatures.empty())
                    text +=
                      " (requires: " + BuildRequiredFeaturesText(result.metadataResult.metadata.requiredFeatures) + ")";
                return {CompatibilityPreviewStatus::Compatible, text};
            }
            if(result.decision.reasonKey == std::string("chaos.compatibility.rules_profile_mismatch"))
                return {CompatibilityPreviewStatus::Incompatible,
                        "Chaos metadata: incompatible (requires a different rules profile)"};
            if(!result.decision.missingRequiredFeatures.empty())
                return {CompatibilityPreviewStatus::Incompatible,
                        "Chaos metadata: incompatible (unsupported features: "
                          + BuildRequiredFeaturesText(result.decision.missingRequiredFeatures) + ")"};
            return {CompatibilityPreviewStatus::Incompatible,
                    "Chaos metadata: incompatible (requirements are not satisfied)"};
    }
    return {CompatibilityPreviewStatus::Incompatible, "Chaos metadata: incompatible (requirements are not satisfied)"};
}

} // namespace chaos
