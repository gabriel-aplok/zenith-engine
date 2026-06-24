#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace Zenith
{
    struct ImportMetadata
    {
        std::string virtualPath;
        std::string sourcePath;
        std::string bakedPath;
        std::string importerName;
        std::uint32_t importerVersion = 0;
        std::uint64_t sourceHash = 0;
        std::string settingsJson;
        std::vector<std::string> dependencies;
        std::int64_t importedAtUtc = 0;
    };

    bool readImportMetadata(const std::filesystem::path &path, ImportMetadata &metadata);
    bool writeImportMetadata(const std::filesystem::path &path, const ImportMetadata &metadata);
} // namespace Zenith
