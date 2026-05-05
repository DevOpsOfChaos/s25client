// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "RulesProfile.h"

#include <algorithm>
#include <array>
#include <vector>

namespace chaos {

enum class FeatureId
{
    RulesProfile,
    ExtendedContent,
    ExtendedAi,
    ExtendedVisuals,
    MapMetadataV1,
    CompatibilityPreviewStatus,
    ToolOrderingDefaultEnabled,
    AutomaticFlagPlacementDefaultEnabled
};

using FeatureList = std::vector<FeatureId>;
using RequiredFeatures = FeatureList;
using SupportedFeatures = FeatureList;

struct FeatureDefinition
{
    FeatureId id;
    const char* stableKey;
    const char* category;
    bool userFacing;
    bool supportedByRttrCompatible;
    bool supportedByChaos;
};

struct CompatibilityDecision
{
    bool allowed;
    RequiredFeatures missingRequiredFeatures;
    const char* reasonKey;
};

inline constexpr std::array<FeatureDefinition, 8> KnownFeatureDefinitions = {{
  {FeatureId::RulesProfile, "chaos.rules_profile", "metadata", true, false, true},
  {FeatureId::ExtendedContent, "chaos.extended_content", "reserved", false, false, false},
  {FeatureId::ExtendedAi, "chaos.extended_ai", "reserved", false, false, false},
  {FeatureId::ExtendedVisuals, "chaos.extended_visuals", "reserved", false, false, false},
  {FeatureId::MapMetadataV1, "chaos.map_metadata_v1", "reserved", false, false, false},
  {FeatureId::CompatibilityPreviewStatus, "chaos.ui.compatibility_preview_status", "ui", true, false, true},
  {FeatureId::ToolOrderingDefaultEnabled, "chaos.rules.tool_ordering_default_enabled", "rules", true, false, true},
  {FeatureId::AutomaticFlagPlacementDefaultEnabled, "chaos.rules.automatic_flag_placement_default_enabled", "rules",
   true, false, true},
}};

inline const std::array<FeatureDefinition, 8>& GetKnownFeatureDefinitions()
{
    return KnownFeatureDefinitions;
}

inline const FeatureDefinition* FindFeatureDefinition(const FeatureId featureId)
{
    const auto& definitions = GetKnownFeatureDefinitions();
    const auto it =
      std::find_if(definitions.begin(), definitions.end(),
                   [featureId](const FeatureDefinition& definition) { return definition.id == featureId; });
    return it == definitions.end() ? nullptr : &*it;
}

inline const char* ToStableFeatureKey(const FeatureId featureId)
{
    if(const FeatureDefinition* definition = FindFeatureDefinition(featureId))
        return definition->stableKey;
    return "chaos.unknown";
}

inline bool ContainsFeature(const FeatureList& features, const FeatureId featureId)
{
    return std::find(features.begin(), features.end(), featureId) != features.end();
}

inline bool IsFeatureSupportedByProfile(const FeatureDefinition& definition, const RulesProfile rulesProfile)
{
    switch(rulesProfile)
    {
        case RulesProfile::RttrCompatible: return definition.supportedByRttrCompatible;
        case RulesProfile::Chaos: return definition.supportedByChaos;
    }
    return false;
}

inline bool IsFeatureSupportedByProfile(const FeatureId featureId, const RulesProfile rulesProfile)
{
    const FeatureDefinition* definition = FindFeatureDefinition(featureId);
    return definition && IsFeatureSupportedByProfile(*definition, rulesProfile);
}

inline SupportedFeatures GetSupportedFeatures(const RulesProfile rulesProfile)
{
    SupportedFeatures supportedFeatures;
    for(const FeatureDefinition& definition : GetKnownFeatureDefinitions())
    {
        if(IsFeatureSupportedByProfile(definition, rulesProfile))
            supportedFeatures.push_back(definition.id);
    }
    return supportedFeatures;
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
