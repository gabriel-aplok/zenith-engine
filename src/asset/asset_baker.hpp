#pragma once

#include <filesystem>

namespace AssetBaker {

    bool bakeModelToAsset(const std::filesystem::path& sourcePath,
                          const std::filesystem::path& destinationPath);

} // namespace AssetBaker
