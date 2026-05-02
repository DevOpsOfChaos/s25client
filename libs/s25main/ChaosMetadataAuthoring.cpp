// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosMetadataAuthoring.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/filesystem.hpp>
#include <boost/nowide/fstream.hpp>
#include <vector>

namespace bfs = boost::filesystem;

namespace chaos {

std::string SerializeRequiredFeatures(const RequiredFeatures& requiredFeatures)
{
    std::vector<std::string> featureKeys;
    featureKeys.reserve(requiredFeatures.size());
    for(const FeatureId featureId : requiredFeatures)
        featureKeys.emplace_back(ToStableFeatureKey(featureId));
    return boost::algorithm::join(featureKeys, ", ");
}

std::string SerializeCompatibilityMetadata(const CompatibilityMetadata& metadata)
{
    std::string output;
    if(metadata.requiredRulesProfile)
    {
        output += "rulesProfile=";
        output += SerializeRulesProfile(*metadata.requiredRulesProfile);
        output += "\n";
    }
    output += "requiredFeatures=";
    output += SerializeRequiredFeatures(metadata.requiredFeatures);
    output += "\n";
    if(!metadata.minChaosVersion.empty())
    {
        output += "minChaosVersion=";
        output += metadata.minChaosVersion;
        output += "\n";
    }
    return output;
}

MetadataWriteResult WriteCompatibilityMetadata(const bfs::path& contentPath, const CompatibilityMetadata& metadata,
                                               const bool overwrite)
{
    const bfs::path metadataPath = GetCompatibilityMetadataPath(contentPath);
    if(bfs::exists(metadataPath) && !overwrite)
        return {MetadataWriteStatus::AlreadyExists, metadataPath,
                "metadata file already exists; pass --overwrite to replace it"};

    boost::nowide::ofstream file(metadataPath.string(), std::ios::out | std::ios::trunc);
    if(!file)
        return {MetadataWriteStatus::Invalid, metadataPath, "could not write metadata file"};

    file << SerializeCompatibilityMetadata(metadata);
    if(!file)
        return {MetadataWriteStatus::Invalid, metadataPath, "could not finish writing metadata file"};

    return {MetadataWriteStatus::Written, metadataPath, "metadata file written"};
}

} // namespace chaos
