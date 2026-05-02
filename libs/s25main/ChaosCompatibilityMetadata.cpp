// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityMetadata.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace bfs = boost::filesystem;

namespace chaos {
namespace {

    std::string Trim(const std::string& value)
    {
        auto begin = value.begin();
        while(begin != value.end() && std::isspace(static_cast<unsigned char>(*begin)))
            ++begin;

        auto end = value.end();
        while(end != begin && std::isspace(static_cast<unsigned char>(*(end - 1))))
            --end;

        return std::string(begin, end);
    }

    std::vector<std::string> SplitCommaSeparatedList(const std::string& value)
    {
        std::vector<std::string> values;
        std::stringstream stream(value);
        std::string item;
        while(std::getline(stream, item, ','))
        {
            item = Trim(item);
            if(!item.empty())
                values.push_back(item);
        }
        return values;
    }

    CompatibilityDecision BuildInvalidMetadataDecision() { return {false, {}, "chaos.compatibility.invalid_metadata"}; }

    std::string JoinMissingFeatures(const RequiredFeatures& features)
    {
        std::vector<std::string> featureKeys;
        featureKeys.reserve(features.size());
        for(const FeatureId featureId : features)
            featureKeys.emplace_back(ToStableFeatureKey(featureId));
        return boost::algorithm::join(featureKeys, ", ");
    }

} // namespace

bfs::path GetCompatibilityMetadataPath(const bfs::path& contentPath)
{
    bfs::path metadataPath = contentPath;
    metadataPath += ".chaos";
    return metadataPath;
}

boost::optional<FeatureId> ParseFeatureId(const std::string& value)
{
    if(value == ToStableFeatureKey(FeatureId::RulesProfile))
        return FeatureId::RulesProfile;
    if(value == ToStableFeatureKey(FeatureId::ExtendedContent))
        return FeatureId::ExtendedContent;
    if(value == ToStableFeatureKey(FeatureId::ExtendedAi))
        return FeatureId::ExtendedAi;
    if(value == ToStableFeatureKey(FeatureId::ExtendedVisuals))
        return FeatureId::ExtendedVisuals;
    if(value == ToStableFeatureKey(FeatureId::MapMetadataV1))
        return FeatureId::MapMetadataV1;
    return boost::none;
}

MetadataReadResult ReadCompatibilityMetadata(const bfs::path& contentPath)
{
    const bfs::path metadataPath = GetCompatibilityMetadataPath(contentPath);
    if(!bfs::is_regular_file(metadataPath))
        return {MetadataReadStatus::Missing, {}, {}};

    boost::nowide::ifstream file(metadataPath.string());
    if(!file)
        return {MetadataReadStatus::Invalid, {}, "could not open metadata file"};

    CompatibilityMetadata metadata;
    std::string line;
    unsigned lineNumber = 0;
    while(std::getline(file, line))
    {
        ++lineNumber;
        line = Trim(line);
        if(line.empty() || line[0] == '#')
            continue;

        const std::string::size_type separator = line.find('=');
        if(separator == std::string::npos)
            return {MetadataReadStatus::Invalid, {}, "line " + std::to_string(lineNumber) + " is missing '='"};

        const std::string key = Trim(line.substr(0, separator));
        const std::string value = Trim(line.substr(separator + 1));
        if(key.empty())
            return {MetadataReadStatus::Invalid, {}, "line " + std::to_string(lineNumber) + " has an empty key"};

        if(key == "rulesProfile")
        {
            const RulesProfile parsedProfile = ParseRulesProfile(value, RulesProfile::Chaos);
            if(value != SerializeRulesProfile(parsedProfile))
                return {MetadataReadStatus::Invalid, {}, "invalid rulesProfile '" + value + "'"};
            metadata.requiredRulesProfile = parsedProfile;
        } else if(key == "requiredFeatures")
        {
            for(const std::string& featureValue : SplitCommaSeparatedList(value))
            {
                const boost::optional<FeatureId> featureId = ParseFeatureId(featureValue);
                if(!featureId)
                    return {MetadataReadStatus::Invalid, {}, "invalid required feature '" + featureValue + "'"};
                if(!ContainsFeature(metadata.requiredFeatures, *featureId))
                    metadata.requiredFeatures.push_back(*featureId);
            }
        } else if(key == "minChaosVersion")
            metadata.minChaosVersion = value;
        else
            return {MetadataReadStatus::Invalid, {}, "unknown key '" + key + "'"};
    }

    return {MetadataReadStatus::Valid, metadata, {}};
}

ContentCompatibilityResult EvaluateContentCompatibility(const bfs::path& contentPath, const RulesProfile rulesProfile)
{
    ContentCompatibilityResult result;
    result.metadataPath = GetCompatibilityMetadataPath(contentPath);
    result.metadataResult = ReadCompatibilityMetadata(contentPath);

    switch(result.metadataResult.status)
    {
        case MetadataReadStatus::Missing: result.decision = {true, {}, "chaos.compatibility.allowed"}; break;
        case MetadataReadStatus::Invalid: result.decision = BuildInvalidMetadataDecision(); break;
        case MetadataReadStatus::Valid:
            if(result.metadataResult.metadata.requiredRulesProfile
               && *result.metadataResult.metadata.requiredRulesProfile != rulesProfile)
                result.decision = {false, {}, "chaos.compatibility.rules_profile_mismatch"};
            else
                result.decision = EvaluateCompatibility(rulesProfile, GetSupportedFeatures(rulesProfile),
                                                        result.metadataResult.metadata.requiredFeatures);
            break;
    }

    result.message = BuildCompatibilityMessage(result);
    return result;
}

std::string BuildCompatibilityMessage(const ContentCompatibilityResult& result)
{
    switch(result.metadataResult.status)
    {
        case MetadataReadStatus::Missing: return "No Chaos compatibility metadata present.";
        case MetadataReadStatus::Invalid: return "Invalid Chaos compatibility metadata: " + result.metadataResult.error;
        case MetadataReadStatus::Valid:
            if(result.decision.allowed)
                return "Chaos compatibility metadata requirements are satisfied.";
            if(result.decision.reasonKey == std::string("chaos.compatibility.rules_profile_mismatch"))
                return "Chaos compatibility metadata requires a different rules profile.";
            if(!result.decision.missingRequiredFeatures.empty())
                return "Missing Chaos compatibility features: "
                       + JoinMissingFeatures(result.decision.missingRequiredFeatures);
            return "Chaos compatibility metadata requirements are not satisfied.";
    }
    return "Unknown Chaos compatibility metadata state.";
}

} // namespace chaos
