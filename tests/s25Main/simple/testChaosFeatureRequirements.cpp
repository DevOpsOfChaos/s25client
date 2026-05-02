// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityStatus.h"

#include <boost/test/unit_test.hpp>
#include <string>

BOOST_AUTO_TEST_SUITE(ChaosFeatureRequirements)

BOOST_AUTO_TEST_CASE(DefaultRulesProfileIsChaos)
{
    BOOST_TEST(static_cast<int>(GetDefaultRulesProfile()) == static_cast<int>(RulesProfile::Chaos));
}

BOOST_AUTO_TEST_CASE(RulesProfileDisplayNamesAreStable)
{
    BOOST_TEST(GetRulesProfileDisplayName(RulesProfile::RttrCompatible) == "RTTR-compatible");
    BOOST_TEST(GetRulesProfileDisplayName(RulesProfile::Chaos) == "Chaos");
}

BOOST_AUTO_TEST_CASE(RulesProfileSerializationRoundtrips)
{
    BOOST_TEST(static_cast<int>(ParseRulesProfile(SerializeRulesProfile(RulesProfile::RttrCompatible)))
               == static_cast<int>(RulesProfile::RttrCompatible));
    BOOST_TEST(static_cast<int>(ParseRulesProfile(SerializeRulesProfile(RulesProfile::Chaos)))
               == static_cast<int>(RulesProfile::Chaos));
}

BOOST_AUTO_TEST_CASE(InvalidRulesProfileFallsBackToDefault)
{
    BOOST_TEST(static_cast<int>(ParseRulesProfile("invalid")) == static_cast<int>(GetDefaultRulesProfile()));
    BOOST_TEST(static_cast<int>(ParseRulesProfile("invalid", RulesProfile::RttrCompatible))
               == static_cast<int>(RulesProfile::RttrCompatible));
}

BOOST_AUTO_TEST_CASE(RulesProfileValuesRemainDistinct)
{
    BOOST_TEST(static_cast<int>(RulesProfile::RttrCompatible) != static_cast<int>(RulesProfile::Chaos));
    BOOST_TEST(std::string(SerializeRulesProfile(RulesProfile::RttrCompatible))
               != SerializeRulesProfile(RulesProfile::Chaos));
}

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

BOOST_AUTO_TEST_CASE(EmptyRequirementsAreAllowed)
{
    const auto decision = chaos::EvaluateCompatibility(RulesProfile::RttrCompatible, {}, {});

    BOOST_TEST(decision.allowed);
    BOOST_TEST(decision.missingRequiredFeatures.empty());
    BOOST_TEST(decision.reasonKey == "chaos.compatibility.allowed");
}

BOOST_AUTO_TEST_CASE(SupportedRequirementsAreAllowed)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::RulesProfile};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::RulesProfile};

    const auto decision = chaos::EvaluateCompatibility(RulesProfile::Chaos, supportedFeatures, requiredFeatures);

    BOOST_TEST(decision.allowed);
    BOOST_TEST(decision.missingRequiredFeatures.empty());
    BOOST_TEST(decision.reasonKey == "chaos.compatibility.allowed");
}

BOOST_AUTO_TEST_CASE(MissingRequirementsAreBlocked)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::RulesProfile};

    const auto decision = chaos::EvaluateCompatibility(RulesProfile::Chaos, {}, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
    BOOST_TEST(decision.reasonKey == "chaos.compatibility.missing_required_features");
}

BOOST_AUTO_TEST_CASE(RttrCompatibleProfileDoesNotSilentlySatisfyChaosRequirements)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::RulesProfile};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::RulesProfile};

    const auto decision =
      chaos::EvaluateCompatibility(RulesProfile::RttrCompatible, supportedFeatures, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
}

BOOST_AUTO_TEST_CASE(CompatibilityDecisionMissingFeaturesAreDeterministicAndUnique)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ExtendedAi, chaos::FeatureId::RulesProfile,
                                                      chaos::FeatureId::ExtendedAi, chaos::FeatureId::MapMetadataV1};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::RulesProfile, chaos::FeatureId::ExtendedAi};

    const auto decision = chaos::EvaluateCompatibility(RulesProfile::Chaos, supportedFeatures, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 2u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0]) == static_cast<int>(chaos::FeatureId::ExtendedAi));
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[1])
               == static_cast<int>(chaos::FeatureId::MapMetadataV1));
}

BOOST_AUTO_TEST_CASE(CompatibilityStatusAllowsEmptyRequirements)
{
    const auto status = chaos::GetCompatibilityStatus(RulesProfile::RttrCompatible);

    BOOST_TEST(static_cast<int>(status.rulesProfile) == static_cast<int>(RulesProfile::RttrCompatible));
    BOOST_TEST(status.requiredFeatures.empty());
    BOOST_TEST(status.decision.allowed);
    BOOST_TEST(status.decision.reasonKey == "chaos.compatibility.allowed");
    BOOST_TEST(chaos::GetCompatibilityStatusDisplayName(status) == "Compatible");
}

BOOST_AUTO_TEST_CASE(CompatibilityStatusBlocksMissingRequirements)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ExtendedAi};

    const auto status = chaos::GetCompatibilityStatus(RulesProfile::Chaos, requiredFeatures);

    BOOST_TEST(!status.decision.allowed);
    BOOST_TEST_REQUIRE(status.decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(status.decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::ExtendedAi));
    BOOST_TEST(chaos::GetCompatibilityStatusDisplayName(status) == "Blocked");
}

BOOST_AUTO_TEST_CASE(CompatibilityStatusReflectsSelectedRulesProfile)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::RulesProfile};

    const auto rttrStatus = chaos::GetCompatibilityStatus(RulesProfile::RttrCompatible, requiredFeatures);
    const auto chaosStatus = chaos::GetCompatibilityStatus(RulesProfile::Chaos, requiredFeatures);

    BOOST_TEST(!rttrStatus.decision.allowed);
    BOOST_TEST(chaosStatus.decision.allowed);
}

BOOST_AUTO_TEST_CASE(CompatibilityStatusMissingFeaturesAreStable)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ExtendedAi, chaos::FeatureId::RulesProfile,
                                                      chaos::FeatureId::ExtendedAi, chaos::FeatureId::MapMetadataV1};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::RulesProfile, chaos::FeatureId::ExtendedAi};

    const auto status = chaos::BuildCompatibilityStatus(RulesProfile::Chaos, supportedFeatures, requiredFeatures);

    BOOST_TEST(!status.decision.allowed);
    BOOST_TEST_REQUIRE(status.decision.missingRequiredFeatures.size() == 2u);
    BOOST_TEST(static_cast<int>(status.decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::ExtendedAi));
    BOOST_TEST(static_cast<int>(status.decision.missingRequiredFeatures[1])
               == static_cast<int>(chaos::FeatureId::MapMetadataV1));
}

BOOST_AUTO_TEST_SUITE_END()
