#pragma once

#include <filesystem>
#include <optional>

#include "asset/baked_mesh_asset.hpp"

namespace AssetLoader {

    std::optional<Zenith::BakedMeshAsset> loadBakedMeshAsset(const std::filesystem::path& assetPath);

} // namespace AssetLoader
