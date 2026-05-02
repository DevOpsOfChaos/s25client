// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityMetadata.h"
#include "ChaosCompatibilityPreview.h"

#include "rttr/test/TmpFolder.hpp"

#include <boost/nowide/fstream.hpp>
#include <boost/test/unit_test.hpp>

namespace {

void WriteMetadata(const boost::filesystem::path& contentPath, const std::string& metadata)
{
    boost::nowide::ofstream file(chaos::GetCompatibilityMetadataPath(contentPath).string());
    file << metadata;
}

} // namespace

BOOST_AUTO_TEST_SUITE(ChaosCompatibilityMetadata)

BOOST_AUTO_TEST_CASE(MissingMetadataMeansNoRequirements)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "plain.swd";

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::RttrCompatible);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Missing));
    BOOST_TEST(result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.allowed");
    BOOST_TEST(result.userMessage.empty());
}

BOOST_AUTO_TEST_CASE(ValidMetadataWithSupportedFeaturesIsCompatible)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "chaos.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile\nminChaosVersion=1\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::Chaos);

    BOOST_TEST(static_cast<int>(result.metadataResult.status) == static_cast<int>(chaos::MetadataReadStatus::Valid));
    BOOST_TEST(result.decision.allowed);
    BOOST_TEST_REQUIRE(result.metadataResult.metadata.requiredFeatures.size() == 1u);
    BOOST_TEST(static_cast<int>(result.metadataResult.metadata.requiredFeatures[0])
               == static_cast<int>(chaos::FeatureId::RulesProfile));
    BOOST_TEST(result.metadataResult.metadata.minChaosVersion == "1");
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
    BOOST_TEST(result.userMessage.find("chaos.compatibility") == std::string::npos);
}

BOOST_AUTO_TEST_CASE(RttrCompatibleProfileBlocksChaosOnlyRequirements)
{
    const rttr::test::TmpFolder tmp;
    const boost::filesystem::path mapPath = tmp / "chaos-only.swd";
    WriteMetadata(mapPath, "rulesProfile=chaos\nrequiredFeatures=chaos.rules_profile\n");

    const auto result = chaos::EvaluateContentCompatibility(mapPath, RulesProfile::RttrCompatible);

    BOOST_TEST(!result.decision.allowed);
    BOOST_TEST(result.decision.reasonKey == "chaos.compatibility.rules_profile_mismatch");
    BOOST_TEST(result.userMessage.find("Chaos Edition cannot start") != std::string::npos);
    BOOST_TEST(result.userMessage.find("different rules profile") != std::string::npos);
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
    BOOST_TEST(result.userMessage.find("Chaos Edition cannot start") != std::string::npos);
    BOOST_TEST(result.userMessage.find(".chaos sidecar file") != std::string::npos);
    BOOST_TEST(result.userMessage.find("chaos.not_real") == std::string::npos);
    BOOST_TEST(result.userMessage.find("chaos.compatibility") == std::string::npos);
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
    BOOST_TEST(preview.text.find("chaos.compatibility") == std::string::npos);
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
    BOOST_TEST(rulesProfilePreview.text.find("chaos.compatibility") == std::string::npos);

    BOOST_TEST(static_cast<int>(missingFeaturePreview.status)
               == static_cast<int>(chaos::CompatibilityPreviewStatus::Incompatible));
    BOOST_TEST(missingFeaturePreview.text == "Incompatible: requires unsupported Chaos compatibility features");
    BOOST_TEST(missingFeaturePreview.text.find("chaos.compatibility") == std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()
