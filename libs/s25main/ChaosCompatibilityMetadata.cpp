// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosCompatibilityMetadata.h"
#include "RTTR_Version.h"

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

    std::string GetProductName() { return rttr::version::GetEditionName(); }

} // namespace

bfs::path GetCompatibilityMetadataPath(const bfs::path& contentPath)
{
    bfs::path metadataPath = contentPath;
    metadataPath += ".chaos";
    return metadataPath;
}

bfs::path GetCompatibilityMetadataContentPath(const bfs::path& metadataPath)
{
    const std::string pathString = metadataPath.string();
    const std::string suffix = ".chaos";
    if(pathString.size() >= suffix.size()
       && pathString.compare(pathString.size() - suffix.size(), suffix.size(), suffix) == 0)
        return bfs::path(pathString.substr(0, pathString.size() - suffix.size()));
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

bool ParseRequiredFeatures(const std::string& value, RequiredFeatures& requiredFeatures, std::string& error)
{
    requiredFeatures.clear();
    for(const std::string& featureValue : SplitCommaSeparatedList(value))
    {
        const boost::optional<FeatureId> featureId = ParseFeatureId(featureValue);
        if(!featureId)
        {
            error = "invalid required feature '" + featureValue + "'";
            return false;
        }
        if(!ContainsFeature(requiredFeatures, *featureId))
            requiredFeatures.push_back(*featureId);
    }
    error.clear();
    return true;
}

MetadataReadResult ReadCompatibilityMetadataFile(const bfs::path& metadataPath)
{
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
            RequiredFeatures parsedFeatures;
            std::string error;
            if(!ParseRequiredFeatures(value, parsedFeatures, error))
                return {MetadataReadStatus::Invalid, {}, error};
            for(const FeatureId featureId : parsedFeatures)
            {
                if(!ContainsFeature(metadata.requiredFeatures, featureId))
                    metadata.requiredFeatures.push_back(featureId);
            }
        } else if(key == "minChaosVersion")
            metadata.minChaosVersion = value;
        else
            return {MetadataReadStatus::Invalid, {}, "unknown key '" + key + "'"};
    }

    return {MetadataReadStatus::Valid, metadata, {}};
}

MetadataReadResult ReadCompatibilityMetadata(const bfs::path& contentPath)
{
    return ReadCompatibilityMetadataFile(GetCompatibilityMetadataPath(contentPath));
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
    result.userMessage = BuildUserFacingCompatibilityError(result);
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

std::string BuildUserFacingCompatibilityError(const ContentCompatibilityResult& result)
{
    if(result.decision.allowed)
        return {};

    const std::string productName = GetProductName();
    switch(result.metadataResult.status)
    {
        case MetadataReadStatus::Missing: return {};
        case MetadataReadStatus::Invalid:
            return productName
                   + " cannot start this map or save because its Chaos compatibility metadata is invalid. Check the "
                     ".chaos sidecar file next to the content.";
        case MetadataReadStatus::Valid:
            if(result.decision.reasonKey == std::string("chaos.compatibility.rules_profile_mismatch"))
                return productName + " cannot start this map or save because it requires a different rules profile.";
            if(!result.decision.missingRequiredFeatures.empty())
                return productName
                       + " cannot start this map or save because it requires unsupported Chaos compatibility features: "
                       + JoinMissingFeatures(result.decision.missingRequiredFeatures) + ".";
            return productName
                   + " cannot start this map or save because its Chaos compatibility requirements are not "
                     "satisfied.";
    }
    return productName
           + " cannot start this map or save because its Chaos compatibility requirements are not "
             "satisfied.";
}

} // namespace chaos
