// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityMetadata.h"
#include "ChaosCompatibilityPreview.h"
#include "ChaosMetadataAuthoring.h"

#include "rttr/test/TmpFolder.hpp"

#include <boost/nowide/fstream.hpp>
#include <boost/test/unit_test.hpp>

namespace {

void WriteMetadata(const boost::filesystem::path& contentPath, const std::string& metadata)
{
    boost::nowide::ofstream file(chaos::GetCompatibilityMetadataPath(contentPath).string());
    file << metadata;
}

void AssertNoInternalCompatibilityReasonKey(const std::string& text)
{
    BOOST_TEST(text.find("chaos.compatibility.") == std::string::npos);
}

} // namespace

BOOST_AUTO_TEST_SUITE(ChaosCompatibilityMetadata)

BOOST_AUTO_TEST_CASE(MissingMetadataMeansNoRequirements)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "plain.swd";

    const auto rttrCompatibleResult = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::RttrCompatible);
    const auto chaosResult = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(rttrCompatibleResult.metadataResult.status)
               == static_cast<int>(chaos::MetadataReadStatus::Missing));
    BOOST_TEST(rttrCompatibleResult.decision.allowed);
    BOOST_TEST(rttrCompatibleResult.decision.reasonKey == "chaos.compatibility.allowed");
    BOOST_TEST(rttrCompatibleResult.userMessage.empty());

    BOOST_TEST(static_cast<int>(chaosResult.metadataResult.status)
               == static_cast<int>(chaos::MetadataReadStatus::Missing));
    BOOST_TEST(chaosResult.decision.allowed);
    BOOST_TEST(chaosResult.decision.reasonKey == "chaos.compatibility.allowed");
    BOOST_TEST(chaosResult.userMessage.empty());
}

BOOST_AUTO_TEST_CASE(ValidMetadataWithSupportedFeaturesIsCompatible)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "chaos.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile, "
                           "chaos.ui.compatibility_preview_status\nminChaosVersion=1\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Valid));
    BOOST_TEST(result.decision.allowed);
    BOOST_TEST_REQUIRE(result.metadataResult.metadata.requiredFeatures.size() == 2u);
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[1])
               == static_cast<int>(chaos::FeatureId::CompatibilityPreviewStatus));
    BOOST_TEST(result.metadataResult.metadata.minChaosVersion == "1");
}

BOOST_AUTO_TEST_CASE(ValidMetadataWithAllCurrentlySupportedChaosFeaturesIsCompatible)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "all-supported.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile, "
                           "chaos.ui.compatibility_preview_status, "
                           "chaos.rules.tool_ordering_default_enabled, "
                           "chaos.rules.automatic_flag_placement_default_enabled\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Valid));
    BOOST_TEST(result.decision.allowed);
    BOOST_TEST(result.decision.missingRequiredFeatures.empty());
    BOOST_TEST(result.userMessage.empty());
    BOOST_TEST_REQUIRE(result.metadataResult.metadata.requiredFeatures.size() == 4u);
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[1])
               == static_cast<int>(chaos::FeatureId::CompatibilityPreviewStatus));
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[2])
               == static_cast<int>(chaos::FeatureId::ToolOrderingDefaultEnabled));
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[3])
               == static_cast<int>(chaos::FeatureId::AutomaticFlagPlacementDefaultEnabled));
}

BOOST_AUTO_TEST_CASE(ValidMetadataWithMissingFeaturesIsBlocked)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "future.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile, chaos.extended_ai\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(!result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.missing_required_features");
    BOOST_TEST_REQUIRE(result.decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(result.decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::ExtendedAi));
    BOOST_TEST(result.message.find("chaos.extended_ai") != std::string::npos);
    BOOST_TEST(result.userMessage.find("Chaos Edition cannot start") != std::string::npos);
    AssertNoInternalCompatibilityReasonKey(result.userMessage);
}

BOOST_AUTO_TEST_CASE(RttrCompatibleProfileBlocksChaosOnlyRequirements)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "chaos-only.swd";
    WriteMetadata(mapPath, "requiredFeatures=chaos.ui.compatibility_preview_status\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::RttrCompatible);

    BOOST_TEST(!result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.missing_required_features");
    BOOST_TEST_REQUIRE(result.decision.missingRequiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(result.decision.missingRequiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::CompatibilityPreviewStatus));
    BOOST_TEST(result.userMessage.find("Chaos Edition cannot start") != std::string::npos);
    BOOST_TEST(result.userMessage.find("chaos.ui.compatibility_preview_status") != std::string::npos);
    AssertNoInternalCompatibilityReasonKey(result.userMessage);
}

BOOST_AUTO_TEST_CASE(InvalidMetadataFailsClosedWithoutCrashing)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "broken.swd";
    WriteMetadata(mapPath, "requiredFeatures=chaos.not_real\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Invalid));
    BOOST_TEST(!result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.invalid_metadata");
    BOOST_TEST(result.metadataResult.error == "invalid required feature 'chaos.not_real'");
    BOOST_TEST(!result.message.empty());
    BOOST_TEST(result.userMessage
               == "Chaos Edition cannot start this map or save because its Chaos compatibility metadata is invalid. "
                  "Check the .chaos sidecar file next to the content.");
    BOOST_TEST(result.userMessage.find("chaos.not_real") == std::string::npos);
    AssertNoInternalCompatibilityReasonKey(result.userMessage);
}

BOOST_AUTO_TEST_CASE(InvalidRulesProfileIsRejected)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "broken-profile.swd";
    WriteMetadata(mapPath, "rulesProfile=almost-chaos\nrequiredFeatures=chaos.rules_profile\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Invalid));
    BOOST_TEST(!result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.invalid_metadata");
    BOOST_TEST(result.metadataResult.error == "invalid rulesProfile 'almost-chaos'");
    AssertNoInternalCompatibilityReasonKey(result.userMessage);
}

BOOST_AUTO_TEST_CASE(InvalidRequiredFeaturesFormatIsRejected)
{
    chaos::RequiredFeatures requiredFeatures;
    std::string error;

    const bool parsed = chaos::ParseRequiredFeatures("chaos.rules_profile;chaos.extended_ai", requiredFeatures, error);

    BOOST_TEST(!parsed);
    BOOST_TEST(requiredFeatures.empty());
    BOOST_TEST(error == "invalid required feature 'chaos.rules_profile;chaos.extended_ai'");
    BOOST_TEST(error.find("chaos.compatibility") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(AuthoringWritesDeterministicMetadata)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "new-map.swd";
    chaos::CompatibilityMetadata metadata;
    metadata.requiredRulesProfile = RulesProfile::Chaos;
    metadata.requiredFeatures = {chaos::FeatureId::RulesProfile, chaos::FeatureId::ExtendedAi};
    metadata.minChaosVersion = "1";

    const auto result = chaos::WriteCompatibilityMetadata(mapPath, metadata, false);
    const auto readResult = chaos::ReadCompatibilityMetadata(mapPath);

    BOOST_TEST(static_cast<int>(result.status) == static_cast<int>(chaos::MetadataWriteStatus::Written));
    BOOST_TEST(result.metadataPath.filename().string() == "new-map.swd.chaos");
    BOOST_TEST(chaos::SerializeCompatibilityMetadata(metadata)
               == "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile, chaos.extended_ai\nminChaosVersion=1\n");
    BOOST_TEST(static_cast<int>(readResult.status) == static_cast<int>(chaos::MetadataReadStatus::Valid));
    BOOST_TEST_REQUIRE(readResult.metadata.requiredFeatures.size() == 2u);
}

BOOST_AUTO_TEST_CASE(AuthoringDoesNotOverwriteExistingMetadataByDefault)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "existing-map.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile\n");
    chaos::CompatibilityMetadata metadata;
    metadata.requiredRulesProfile = RulesProfile::RttrCompatible;
    metadata.requiredFeatures = {};

    const auto result = chaos::WriteCompatibilityMetadata(mapPath, metadata, false);
    const auto readResult = chaos::ReadCompatibilityMetadata(mapPath);

    BOOST_TEST(static_cast<int>(result.status) == static_cast<int>(chaos::MetadataWriteStatus::AlreadyExists));
    BOOST_TEST(readResult.metadata.requiredRulesProfile.has_value());
    BOOST_TEST(static_cast<int>(*readResult.metadata.requiredRulesProfile) == static_cast<int>(RulesProfile::Chaos));
    BOOST_TEST_REQUIRE(readResult.metadata.requiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(readResult.metadata.requiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
}

BOOST_AUTO_TEST_CASE(ExistingSaveWithoutMetadataIsUnaffectedAtHelperLevel)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path savePath = tmp / "existing.sav";

    const auto result = chaos::EvaluateContentCompatibility(savePath, RulesProfile::RttrCompatible);

    BOOST_TEST(result.decision.allowed);
    BOOST_TEST(result.metadataPath.filename().string() == "existing.sav.chaos");
}

BOOST_AUTO_TEST_CASE(MissingMetadataBuildsNeutralPreviewStatus)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "plain.swd";

    const auto preview =
      chaos::BuildCompatibilityPreview(chaos::EvaluateContentCompatibility(mapPath, RulesProfile::RttrCompatible));

    BOOST_TEST(static_cast<int>(preview.status) == static_cast<int>(chaos::CompatibilityPreviewStatus::Neutral));
    BOOST_TEST(preview.text == "RTTR-compatible / no Chaos metadata");
    AssertNoInternalCompatibilityReasonKey(preview.text);
}

BOOST_AUTO_TEST_CASE(ValidCompatibleMetadataBuildsChaosCompatiblePreviewStatus)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "chaos.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile\n");

    const auto preview =
      chaos::BuildCompatibilityPreview(chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos));

    BOOST_TEST(static_cast<int>(preview.status) == static_cast<int>(chaos::CompatibilityPreviewStatus::Compatible));
    BOOST_TEST(preview.text == "Chaos Edition compatible");
    AssertNoInternalCompatibilityReasonKey(preview.text);
}

BOOST_AUTO_TEST_CASE(InvalidMetadataBuildsUserFacingPreviewStatusWithoutReasonKeys)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "broken.swd";
    WriteMetadata(mapPath, "requiredFeatures=chaos.not_real\n");

    const auto preview =
      chaos::BuildCompatibilityPreview(chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos));

    BOOST_TEST(static_cast<int>(preview.status) == static_cast<int>(chaos::CompatibilityPreviewStatus::Invalid));
    BOOST_TEST(preview.text == "Invalid Chaos metadata");
    AssertNoInternalCompatibilityReasonKey(preview.text);
}

BOOST_AUTO_TEST_CASE(IncompatibleMetadataBuildsUserFacingPreviewStatusWithoutReasonKeys)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path rulesProfilePath = tmp / "profile.swd";
    const boost::filesystem::path missingFeaturePath = tmp / "features.swd";
    WriteMetadata(rulesProfilePath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile\n");
    WriteMetadata(missingFeaturePath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile, chaos.extended_ai\n");

    const auto rulesProfilePreview = chaos::BuildCompatibilityPreview(
      chaos::EvaluateContentCompatibility(rulesProfilePath, RulesProfile::RttrCompatible));
    const auto missingFeaturePreview =
      chaos::BuildCompatibilityPreview(chaos::EvaluateContentCompatibility(missingFeaturePath, RulesProfile::Chaos));

    BOOST_TEST(static_cast<int>(rulesProfilePreview.status)
               == static_cast<int>(chaos::CompatibilityPreviewStatus::Incompatible));
    BOOST_TEST(rulesProfilePreview.text == "Incompatible: requires a different rules profile");
    AssertNoInternalCompatibilityReasonKey(rulesProfilePreview.text);

    BOOST_TEST(static_cast<int>(missingFeaturePreview.status)
               == static_cast<int>(chaos::CompatibilityPreviewStatus::Incompatible));
    BOOST_TEST(missingFeaturePreview.text == "Incompatible: requires unsupported Chaos compatibility features");
    AssertNoInternalCompatibilityReasonKey(missingFeaturePreview.text);
}

BOOST_AUTO_TEST_SUITE_END()
