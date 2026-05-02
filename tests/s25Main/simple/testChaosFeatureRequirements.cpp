// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosFeatureRequirements.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ChaosFeatureRequirements)

BOOST_AUTO_TEST_CASE(FeatureIdsHaveStableKeys)
{
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::RulesProfile) == "chaos.rules_profile");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::ExtendedContent) == "chaos.extended_content");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::ExtendedAi) == "chaos.extended_ai");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::ExtendedVisuals) == "chaos.extended_visuals");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::MapMetadataV1) == "chaos.map_metadata_v1");
}

BOOST_AUTO_TEST_CASE(CurrentSupportedFeaturesAreMinimal)
{
    const auto rttrCompatibleFeatures = chaos::GetSupportedFeatures(RulesProfile::RttrCompatible);
    const auto chaosFeatures = chaos::GetSupportedFeatures(RulesProfile::Chaos);

    BOOST_TEST(rttrCompatibleFeatures.empty());
    BOOST_TEST_REQUIRE(chaosFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(chaosFeatures[0]) == static_cast<int>(chaos::FeatureId::RulesProfile));
}

BOOST_AUTO_TEST_CASE(MissingFeatureRequirementsAreDeterministicAndUnique)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ExtendedAi, chaos::FeatureId::RulesProfile,
                                                      chaos::FeatureId::ExtendedAi, chaos::FeatureId::MapMetadataV1};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::RulesProfile};

    const auto missingFeatures = chaos::GetMissingFeatureRequirements(requiredFeatures, supportedFeatures);

    BOOST_TEST_REQUIRE(missingFeatures.size() == 2u);
    BOOST_TEST(static_cast<int>(missingFeatures[0]) == static_cast<int>(chaos::FeatureId::ExtendedAi));
    BOOST_TEST(static_cast<int>(missingFeatures[1]) == static_cast<int>(chaos::FeatureId::MapMetadataV1));
}

BOOST_AUTO_TEST_SUITE_END()
