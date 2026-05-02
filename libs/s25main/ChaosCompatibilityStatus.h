// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ChaosFeatureRequirements.h"
#include "RulesProfile.h"

namespace chaos {

struct CompatibilityStatus
{
    RulesProfile rulesProfile;
    SupportedFeatures supportedFeatures;
    RequiredFeatures requiredFeatures;
    CompatibilityDecision decision;
};

inline CompatibilityStatus BuildCompatibilityStatus(const RulesProfile rulesProfile,
                                                    const SupportedFeatures& supportedFeatures,
                                                    const RequiredFeatures& requiredFeatures)
{
    return {rulesProfile, GetSupportedFeaturesForDecision(rulesProfile, supportedFeatures), requiredFeatures,
            EvaluateCompatibility(rulesProfile, supportedFeatures, requiredFeatures)};
}

inline CompatibilityStatus GetCompatibilityStatus(const RulesProfile rulesProfile,
                                                  const RequiredFeatures& requiredFeatures = {})
{
    return BuildCompatibilityStatus(rulesProfile, GetSupportedFeatures(rulesProfile), requiredFeatures);
}

inline const char* GetCompatibilityStatusDisplayName(const CompatibilityStatus& status)
{
    return status.decision.allowed ? "Compatible" : "Blocked";
}

} // namespace chaos
