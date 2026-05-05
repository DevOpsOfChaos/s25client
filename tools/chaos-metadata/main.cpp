// Copyright (C) 2005 - 2026 Settlers Freaks (sf-team at siedler25.org)
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ChaosMetadataAuthoring.h"

#include <boost/filesystem.hpp>
#include <boost/nowide/args.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/program_options.hpp>
#include <exception>
#include <string>

namespace bfs = boost::filesystem;
namespace bnw = boost::nowide;
namespace po = boost::program_options;

namespace {

enum ExitCode
{
    ExitSuccess = 0,
    ExitInvalid = 1,
    ExitIncompatible = 2
};

void PrintResult(const std::string& status, const bfs::path& path, const std::string& message)
{
    bnw::cout << "status=" << status << "\n";
    bnw::cout << "path=" << path.string() << "\n";
    bnw::cout << "message=" << message << "\n";
}

void PrintMetadataDetails(const chaos::CompatibilityMetadata& metadata)
{
    bnw::cout << "rulesProfile="
              << (metadata.requiredRulesProfile ? SerializeRulesProfile(*metadata.requiredRulesProfile) : "unspecified")
              << "\n";
    bnw::cout << "requiredFeatures=" << chaos::BuildRequiredFeaturesText(metadata.requiredFeatures) << "\n";
    bnw::cout << "minChaosVersion=" << (metadata.minChaosVersion.empty() ? "unspecified" : metadata.minChaosVersion)
              << "\n";
}

bool ParseRulesProfileStrict(const std::string& value, RulesProfile& rulesProfile)
{
    rulesProfile = ParseRulesProfile(value, RulesProfile::Chaos);
    return value == SerializeRulesProfile(rulesProfile);
}

bfs::path NormalizeContentPath(const bfs::path& path)
{
    if(path.extension() == ".chaos")
        return chaos::GetCompatibilityMetadataContentPath(path);
    return path;
}

int ValidateMetadata(const bfs::path& inputPath, const RulesProfile rulesProfile)
{
    const bfs::path contentPath = NormalizeContentPath(inputPath);
    const auto result = chaos::EvaluateContentCompatibility(contentPath, rulesProfile);

    if(result.metadataResult.status == chaos::MetadataReadStatus::Missing)
    {
        PrintResult("invalid", result.metadataPath, "Chaos compatibility metadata file does not exist.");
        return ExitInvalid;
    }
    if(result.metadataResult.status == chaos::MetadataReadStatus::Invalid)
    {
        PrintResult("invalid", result.metadataPath, result.message);
        return ExitInvalid;
    }
    if(!result.decision.allowed)
    {
        PrintResult("incompatible", result.metadataPath,
                    result.userMessage.empty() ? result.message : result.userMessage);
        PrintMetadataDetails(result.metadataResult.metadata);
        return ExitIncompatible;
    }

    PrintResult("success", result.metadataPath, result.message);
    PrintMetadataDetails(result.metadataResult.metadata);
    return ExitSuccess;
}

int CreateMetadata(const bfs::path& contentPath, const std::string& rulesProfileValue,
                   const std::string& requiredFeaturesValue, const std::string& minChaosVersion, const bool overwrite)
{
    chaos::CompatibilityMetadata metadata;
    RulesProfile rulesProfile;
    if(!ParseRulesProfileStrict(rulesProfileValue, rulesProfile))
    {
        PrintResult("invalid", chaos::GetCompatibilityMetadataPath(contentPath),
                    "invalid rulesProfile '" + rulesProfileValue + "'");
        return ExitInvalid;
    }
    metadata.requiredRulesProfile = rulesProfile;

    std::string error;
    if(!chaos::ParseRequiredFeatures(requiredFeaturesValue, metadata.requiredFeatures, error))
    {
        PrintResult("invalid", chaos::GetCompatibilityMetadataPath(contentPath), error);
        return ExitInvalid;
    }
    metadata.minChaosVersion = minChaosVersion;

    const auto result = chaos::WriteCompatibilityMetadata(contentPath, metadata, overwrite);
    if(result.status != chaos::MetadataWriteStatus::Written)
    {
        PrintResult("invalid", result.metadataPath, result.message);
        return ExitInvalid;
    }

    PrintResult("success", result.metadataPath, result.message);
    PrintMetadataDetails(metadata);
    return ExitSuccess;
}

std::string GetSupportedProfiles(const chaos::FeatureDefinition& definition)
{
    std::string profiles;
    if(definition.supportedByRttrCompatible)
        profiles += SerializeRulesProfile(RulesProfile::RttrCompatible);
    if(definition.supportedByChaos)
    {
        if(!profiles.empty())
            profiles += ",";
        profiles += SerializeRulesProfile(RulesProfile::Chaos);
    }
    return profiles.empty() ? "none" : profiles;
}

int ListFeatures()
{
    PrintResult("success", {}, "Registered Chaos compatibility features.");
    bnw::cout << "features:\n";
    bnw::cout << "key\tcategory\tsupportedProfiles\tuserFacing\n";
    for(const chaos::FeatureDefinition& definition : chaos::GetKnownFeatureDefinitions())
    {
        bnw::cout << definition.stableKey << "\t" << definition.category << "\t" << GetSupportedProfiles(definition)
                  << "\t" << (definition.userFacing ? "yes" : "no") << "\n";
    }
    return ExitSuccess;
}

void PrintHelp(const po::options_description& options)
{
    bnw::cout << "Usage:\n";
    bnw::cout << "  chaos-metadata validate <content-or-chaos-path> [--rules-profile <profile>]\n";
    bnw::cout << "  chaos-metadata create <content-path> --rules-profile <profile> --required-features <features>"
                 " [--min-chaos-version <version>] [--overwrite]\n\n";
    bnw::cout << "  chaos-metadata features\n\n";
    bnw::cout << options << "\n";
}

} // namespace

int main(int argc, char** argv)
{
    bnw::args _(argc, argv);

    po::options_description options("Options");
    options.add_options()("help,h", "show help")("command", po::value<std::string>(), "validate, create, or features")(
      "path", po::value<std::string>(), "content file or .chaos metadata path")(
      "rules-profile", po::value<std::string>()->default_value("chaos"), "rules profile: chaos or rttr-compatible")(
      "required-features", po::value<std::string>(), "comma-separated stable feature keys")(
      "min-chaos-version", po::value<std::string>()->default_value(""),
      "optional minimum Chaos version")("overwrite", "replace an existing .chaos metadata file");

    po::positional_options_description positional;
    positional.add("command", 1);
    positional.add("path", 1);

    po::variables_map values;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(options).positional(positional).run(), values);
        po::notify(values);
    } catch(const std::exception& e)
    {
        PrintResult("invalid", {}, e.what());
        return ExitInvalid;
    }

    if(values.count("help") || !values.count("command"))
    {
        PrintHelp(options);
        return values.count("help") ? ExitSuccess : ExitInvalid;
    }

    const std::string command = values["command"].as<std::string>();
    if(command == "features")
        return ListFeatures();

    if(!values.count("path"))
    {
        PrintHelp(options);
        return ExitInvalid;
    }

    const bfs::path path(values["path"].as<std::string>());

    RulesProfile rulesProfile;
    const std::string rulesProfileValue = values["rules-profile"].as<std::string>();
    if(!ParseRulesProfileStrict(rulesProfileValue, rulesProfile))
    {
        PrintResult("invalid", path, "invalid rulesProfile '" + rulesProfileValue + "'");
        return ExitInvalid;
    }

    if(command == "validate")
        return ValidateMetadata(path, rulesProfile);
    if(command == "create")
    {
        if(!values.count("required-features"))
        {
            PrintResult("invalid", chaos::GetCompatibilityMetadataPath(path), "requiredFeatures must be provided");
            return ExitInvalid;
        }
        return CreateMetadata(path, rulesProfileValue, values["required-features"].as<std::string>(),
                              values["min-chaos-version"].as<std::string>(), values.count("overwrite") != 0);
    }

    PrintResult("invalid", path, "unknown command '" + command + "'");
    return ExitInvalid;
}
