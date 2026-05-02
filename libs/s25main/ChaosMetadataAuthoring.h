// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include "ChaosCompatibilityMetadata.h"

#include <boost/filesystem/path.hpp>
#include <string>

namespace chaos {

enum class MetadataWriteStatus
{
    Written,
    AlreadyExists,
    Invalid
};

struct MetadataWriteResult
{
    MetadataWriteStatus status;
    boost::filesystem::path metadataPath;
    std::string message;
};

std::string SerializeRequiredFeatures(const RequiredFeatures& requiredFeatures);
std::string SerializeCompatibilityMetadata(const CompatibilityMetadata& metadata);
MetadataWriteResult WriteCompatibilityMetadata(const boost::filesystem::path& contentPath,
                                               const CompatibilityMetadata& metadata, bool overwrite);

} // namespace chaos
