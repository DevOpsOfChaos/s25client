// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ChaosCompatibilityStatus.h"
#include "RulesProfile.h"

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>
#include <string>

namespace chaos {

enum class MetadataReadStatus
{
    Missing,
    Valid,
    Invalid
};

struct CompatibilityMetadata
{
    boost::optional<RulesProfile> requiredRulesProfile;
    RequiredFeatures requiredFeatures;
    std::string minChaosVersion;
};

struct MetadataReadResult
{
    MetadataReadStatus status;
    CompatibilityMetadata metadata;
    std::string error;
};

struct ContentCompatibilityResult
{
    boost::filesystem::path metadataPath;
    MetadataReadResult metadataResult;
    CompatibilityDecision decision;
    std::string message;
};

boost::filesystem::path GetCompatibilityMetadataPath(const boost::filesystem::path& contentPath);
MetadataReadResult ReadCompatibilityMetadata(const boost::filesystem::path& contentPath);
ContentCompatibilityResult EvaluateContentCompatibility(const boost::filesystem::path& contentPath,
                                                        RulesProfile rulesProfile);
boost::optional<FeatureId> ParseFeatureId(const std::string& value);
std::string BuildCompatibilityMessage(const ContentCompatibilityResult& result);

} // namespace chaos
