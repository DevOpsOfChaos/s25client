// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityMetadata.h"
#include "ChaosCompatibilityStatus.h"
#include "GlobalGameSettings.h"
#include "addons/const_addons.h"

#include <boost/test/unit_test.hpp>
#include <set>
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
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::CompatibilityPreviewStatus)
               == "chaos.ui.compatibility_preview_status");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::ToolOrderingDefaultEnabled)
               == "chaos.rules.tool_ordering_default_enabled");
    BOOST_TEST(chaos::ToStableFeatureKey(chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled)
               == "chaos.rules.automatic_flag_placement_default_enabled");
}

BOOST_AUTO_TEST_CASE(FeatureDefinitionsHaveUniqueChaosPrefixedKeys)
{
    std::set<std::string> knownKeys;
    for(const chaos::FeatureDefinition& definition : chaos::GetKnownFeatureDefinitions())
    {
        const std::string stableKey = definition.stableKey;
        BOOST_TEST(stableKey.find("chaos.") == 0u);
        BOOST_TEST(knownKeys.insert(stableKey).second);
    }
}

BOOST_AUTO_TEST_CASE(FeatureDefinitionsKeepDeterministicFeaturesListOrder)
{
    const auto& definitions = chaos::GetKnownFeatureDefinitions();

    BOOST_TEST_REQUIRE(definitions.size() == 8u);
    BOOST_TEST(definitions[0].stableKey == std::string("chaos.rules_profile"));
    BOOST_TEST(definitions[1].stableKey == std::string("chaos.extended_content"));
    BOOST_TEST(definitions[2].stableKey == std::string("chaos.extended_ai"));
    BOOST_TEST(definitions[3].stableKey == std::string("chaos.extended_visuals"));
    BOOST_TEST(definitions[4].stableKey == std::string("chaos.map_metadata_v1"));
    BOOST_TEST(definitions[5].stableKey == std::string("chaos.ui.compatibility_preview_status"));
    BOOST_TEST(definitions[6].stableKey == std::string("chaos.rules.tool_ordering_default_enabled"));
    BOOST_TEST(definitions[7].stableKey == std::string("chaos.rules.automatic_flag_placement_default_enabled"));
}

BOOST_AUTO_TEST_CASE(CompatibilityPreviewStatusIsRegisteredCentralFeature)
{
    const chaos::FeatureDefinition* definition =
      chaos::FindFeatureDefinition(chaos::FeatureId::CompatibilityPreviewStatus);

    BOOST_TEST_REQUIRE(definition != nullptr);
    BOOST_TEST(definition->stableKey == std::string("chaos.ui.compatibility_preview_status"));
    BOOST_TEST(definition->category == std::string("ui"));
    BOOST_TEST(definition->userFacing);
}

BOOST_AUTO_TEST_CASE(ToolOrderingDefaultEnabledIsRegisteredCentralFeature)
{
    const chaos::FeatureDefinition* definition =
      chaos::FindFeatureDefinition(chaos::FeatureId::ToolOrderingDefaultEnabled);

    BOOST_TEST_REQUIRE(definition != nullptr);
    BOOST_TEST(definition->stableKey == std::string("chaos.rules.tool_ordering_default_enabled"));
    BOOST_TEST(definition->category == std::string("rules"));
    BOOST_TEST(definition->userFacing);
}

BOOST_AUTO_TEST_CASE(AutomaticFlagPlacementDefaultEnabledIsRegisteredCentralFeature)
{
    const chaos::FeatureDefinition* definition =
      chaos::FindFeatureDefinition(chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled);

    BOOST_TEST_REQUIRE(definition != nullptr);
    BOOST_TEST(definition->stableKey == std::string("chaos.rules.automatic_flag_placement_default_enabled"));
    BOOST_TEST(definition->category == std::string("rules"));
    BOOST_TEST(definition->userFacing);
}

BOOST_AUTO_TEST_CASE(ParserAcceptsEveryRegisteredFeatureKey)
{
    for(const chaos::FeatureDefinition& definition : chaos::GetKnownFeatureDefinitions())
    {
        chaos::RequiredFeatures requiredFeatures;
        std::string error;

        const bool parsed = chaos::ParseRequiredFeatures(definition.stableKey, requiredFeatures, error);

        BOOST_TEST(parsed);
        BOOST_TEST(error.empty());
        BOOST_TEST_REQUIRE(requiredFeatures.size() == 1u);
        BOOST_TEST(static_cast<int>(requiredFeatures[0]) == static_cast<int>(definition.id));
    }
}

BOOST_AUTO_TEST_CASE(FeatureDefinitionsDeclareUserFacingSurface)
{
    for(const chaos::FeatureDefinition& definition : chaos::GetKnownFeatureDefinitions())
    {
        const bool expectedUserFacing = definition.id == chaos::FeatureId::RulesProfile
                                        || definition.id == chaos::FeatureId::CompatibilityPreviewStatus
                                        || definition.id == chaos::FeatureId::ToolOrderingDefaultEnabled
                                        || definition.id == chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled;

        BOOST_TEST(definition.userFacing == expectedUserFacing);
    }
}

BOOST_AUTO_TEST_CASE(CurrentSupportedFeaturesAreChaosOnlyAndMinimal)
{
    const auto rttrCompatibleFeatures = chaos::GetSupportedFeatures(RulesProfile::RttrCompatible);
    const auto chaosFeatures = chaos::GetSupportedFeatures(RulesProfile::Chaos);

    BOOST_TEST(rttrCompatibleFeatures.empty());
    BOOST_TEST_REQUIRE(chaosFeatures.size() == 4u);
    BOOST_TEST(static_cast<int>(chaosFeatures[0]) == static_cast<int>(chaos::FeatureId::RulesProfile));
    BOOST_TEST(static_cast<int>(chaosFeatures[1]) == static_cast<int>(chaos::FeatureId::CompatibilityPreviewStatus));
    BOOST_TEST(static_cast<int>(chaosFeatures[2]) == static_cast<int>(chaos::FeatureId::ToolOrderingDefaultEnabled));
    BOOST_TEST(static_cast<int>(chaosFeatures[3])
               == static_cast<int>(chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled));
}

BOOST_AUTO_TEST_CASE(ProfileSupportIsReadFromRegisteredFeatureDefinitions)
{
    const chaos::FeatureDefinition* definition =
      chaos::FindFeatureDefinition(chaos::FeatureId::CompatibilityPreviewStatus);

    BOOST_TEST_REQUIRE(definition != nullptr);
    BOOST_TEST(chaos::IsFeatureSupportedByProfile(*definition, RulesProfile::Chaos));
    BOOST_TEST(!chaos::IsFeatureSupportedByProfile(*definition, RulesProfile::RttrCompatible));
}

BOOST_AUTO_TEST_CASE(ToolOrderingDefaultEnabledRequirementIsAllowedForChaosProfile)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ToolOrderingDefaultEnabled};

    const auto decision = chaos::EvaluateCompatibility(
      RulesProfile::Chaos, chaos::GetSupportedFeatures(RulesProfile::Chaos), requiredFeatures);

    BOOST_TEST(decision.allowed);
    BOOST_TEST(decision.missingRequiredFeatures.empty());
    BOOST_TEST(decision.reasonKey == "chaos.compatibility.allowed");
}

BOOST_AUTO_TEST_CASE(RttrCompatibleProfileDoesNotSupportToolOrderingDefaultEnabled)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ToolOrderingDefaultEnabled};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::ToolOrderingDefaultEnabled};

    const auto decision =
      chaos::EvaluateCompatibility(RulesProfile::RttrCompatible, supportedFeatures, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::ToolOrderingDefaultEnabled));
}

BOOST_AUTO_TEST_CASE(ChaosRuleDefaultsEnableToolOrderingAndAutomaticFlagPlacement)
{
    const GlobalGameSettings rttrCompatibleSettings(RulesProfile::RttrCompatible);
    const GlobalGameSettings chaosSettings(RulesProfile::Chaos);

    BOOST_TEST(!rttrCompatibleSettings.isEnabled(AddonId::TOOL_ORDERING));
    BOOST_TEST(rttrCompatibleSettings.getSelection(AddonId::TOOL_ORDERING) == 0u);
    BOOST_TEST(!rttrCompatibleSettings.isEnabled(AddonId::AUTOFLAGS));
    BOOST_TEST(rttrCompatibleSettings.getSelection(AddonId::AUTOFLAGS) == 0u);

    BOOST_TEST(chaosSettings.isEnabled(AddonId::TOOL_ORDERING));
    BOOST_TEST(chaosSettings.getSelection(AddonId::TOOL_ORDERING) == 1u);
    BOOST_TEST(chaosSettings.isEnabled(AddonId::AUTOFLAGS));
    BOOST_TEST(chaosSettings.getSelection(AddonId::AUTOFLAGS) == 1u);
}

BOOST_AUTO_TEST_CASE(AutomaticFlagPlacementDefaultEnabledRequirementIsAllowedForChaosProfile)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled};

    const auto decision = chaos::EvaluateCompatibility(
      RulesProfile::Chaos, chaos::GetSupportedFeatures(RulesProfile::Chaos), requiredFeatures);

    BOOST_TEST(decision.allowed);
    BOOST_TEST(decision.missingRequiredFeatures.empty());
    BOOST_TEST(decision.reasonKey == "chaos.compatibility.allowed");
}

BOOST_AUTO_TEST_CASE(RttrCompatibleProfileDoesNotSupportAutomaticFlagPlacementDefaultEnabled)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled};

    const auto decision =
      chaos::EvaluateCompatibility(RulesProfile::RttrCompatible, supportedFeatures, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled));
}

BOOST_AUTO_TEST_CASE(ExplicitAddonConfigurationOverridesChaosRuleDefaults)
{
    GlobalGameSettings settings(RulesProfile::Chaos);
    settings.setSelection(AddonId::TOOL_ORDERING, 0);
    settings.setSelection(AddonId::AUTOFLAGS, 0);

    BOOST_TEST(!settings.isEnabled(AddonId::TOOL_ORDERING));
    BOOST_TEST(settings.getSelection(AddonId::TOOL_ORDERING) == 0u);
    BOOST_TEST(!settings.isEnabled(AddonId::AUTOFLAGS));
    BOOST_TEST(settings.getSelection(AddonId::AUTOFLAGS) == 0u);
}

BOOST_AUTO_TEST_CASE(RequiredFeaturesDoNotApplyAddonConfiguration)
{
    GlobalGameSettings settings(RulesProfile::RttrCompatible);
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::ToolOrderingDefaultEnabled,
                                                      chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled};

    const auto decision = chaos::EvaluateCompatibility(
      RulesProfile::Chaos, chaos::GetSupportedFeatures(RulesProfile::Chaos), requiredFeatures);

    BOOST_TEST(decision.allowed);
    BOOST_TEST(!settings.isEnabled(AddonId::TOOL_ORDERING));
    BOOST_TEST(settings.getSelection(AddonId::TOOL_ORDERING) == 0u);
    BOOST_TEST(!settings.isEnabled(AddonId::AUTOFLAGS));
    BOOST_TEST(settings.getSelection(AddonId::AUTOFLAGS) == 0u);
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

BOOST_AUTO_TEST_CASE(CompatibilityPreviewStatusRequirementIsAllowedForChaosProfile)
{
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::CompatibilityPreviewStatus};

    const auto decision = chaos::EvaluateCompatibility(
      RulesProfile::Chaos, chaos::GetSupportedFeatures(RulesProfile::Chaos), requiredFeatures);

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
    const chaos::RequiredFeatures requiredFeatures = {chaos::FeatureId::CompatibilityPreviewStatus};
    const chaos::SupportedFeatures supportedFeatures = {chaos::FeatureId::CompatibilityPreviewStatus};

    const auto decision =
      chaos::EvaluateCompatibility(RulesProfile::RttrCompatible, supportedFeatures, requiredFeatures);

    BOOST_TEST(!decision.allowed);
    BOOST_TEST_REQUIRE(decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::CompatibilityPreviewStatus));
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
