// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "RulesProfile.h"

#include <algorithm>
#include <vector>

namespace chaos {

enum class FeatureId
{
    RulesProfile,
    ExtendedContent,
    ExtendedAi,
    ExtendedVisuals,
    MapMetadataV1,
    CompatibilityPreviewStatus
};

using FeatureList = std::vector<FeatureId>;
using RequiredFeatures = FeatureList;
using SupportedFeatures = FeatureList;

struct CompatibilityDecision
{
    bool allowed;
    RequiredFeatures missingRequiredFeatures;
    const char* reasonKey;
};

inline const char* ToStableFeatureKey(const FeatureId featureId)
{
    switch(featureId)
    {
        case FeatureId::RulesProfile: return "chaos.rules_profile";
        case FeatureId::ExtendedContent: return "chaos.extended_content";
        case FeatureId::ExtendedAi: return "chaos.extended_ai";
        case FeatureId::ExtendedVisuals: return "chaos.extended_visuals";
        case FeatureId::MapMetadataV1: return "chaos.map_metadata_v1";
        case FeatureId::CompatibilityPreviewStatus: return "chaos.ui.compatibility_preview_status";
    }
    return "chaos.unknown";
}

inline bool ContainsFeature(const FeatureList& features, const FeatureId featureId)
{
    return std::find(features.begin(), features.end(), featureId) != features.end();
}

inline SupportedFeatures GetSupportedFeatures(const RulesProfile rulesProfile)
{
    switch(rulesProfile)
    {
        case RulesProfile::RttrCompatible: return {};
        case RulesProfile::Chaos: return {FeatureId::RulesProfile, FeatureId::CompatibilityPreviewStatus};
    }
    return {};
}

inline SupportedFeatures GetSupportedFeaturesForDecision(const RulesProfile rulesProfile,
                                                         const SupportedFeatures& supportedFeatures)
{
    const SupportedFeatures profileSupportedFeatures = GetSupportedFeatures(rulesProfile);
    SupportedFeatures effectiveSupportedFeatures;
    for(const FeatureId featureId : supportedFeatures)
    {
        if(ContainsFeature(profileSupportedFeatures, featureId)
           && !ContainsFeature(effectiveSupportedFeatures, featureId))
            effectiveSupportedFeatures.push_back(featureId);
    }
    return effectiveSupportedFeatures;
}

inline RequiredFeatures GetMissingFeatureRequirements(const RequiredFeatures& requiredFeatures,
                                                      const SupportedFeatures& supportedFeatures)
{
    RequiredFeatures missingFeatures;
    for(const FeatureId featureId : requiredFeatures)
    {
        if(!ContainsFeature(supportedFeatures, featureId) && !ContainsFeature(missingFeatures, featureId))
            missingFeatures.push_back(featureId);
    }
    return missingFeatures;
}

inline CompatibilityDecision EvaluateCompatibility(const RulesProfile rulesProfile,
                                                   const SupportedFeatures& supportedFeatures,
                                                   const RequiredFeatures& requiredFeatures)
{
    const SupportedFeatures effectiveSupportedFeatures =
      GetSupportedFeaturesForDecision(rulesProfile, supportedFeatures);
    RequiredFeatures missingFeatures = GetMissingFeatureRequirements(requiredFeatures, effectiveSupportedFeatures);
    const bool allowed = missingFeatures.empty();
    return {allowed, missingFeatures,
            allowed ? "chaos.compatibility.allowed" : "chaos.compatibility.missing_required_features"};
}

} // namespace chaos
