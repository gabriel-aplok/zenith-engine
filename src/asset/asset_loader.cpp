#include "asset/asset_loader.hpp"

#include "log/log.hpp"

#include <fstream>

namespace {

    struct AssetHeader {
        std::uint64_t vertexCount = 0;
        std::uint64_t indexCount = 0;
    };

} // namespace

std::optional<Zenith::BakedMeshAsset> AssetLoader::loadBakedMeshAsset(const std::filesystem::path& assetPath) {
    std::ifstream in(assetPath, std::ios::binary);
    if (!in.is_open()) {
        Zenith::Log::Warn("Cannot open asset file: {}", assetPath.string());
        return std::nullopt;
    }

    AssetHeader header;
    in.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.vertexCount == 0 || header.indexCount == 0) {
        Zenith::Log::Warn("Asset is empty or corrupted: {}", assetPath.string());
        return std::nullopt;
    }

    Zenith::BakedMeshAsset asset;
    asset.vertices.resize(static_cast<std::size_t>(header.vertexCount));
    in.read(reinterpret_cast<char*>(asset.vertices.data()),
            static_cast<std::streamsize>(header.vertexCount * sizeof(float)));

    asset.indices.resize(static_cast<std::size_t>(header.indexCount));
    in.read(reinterpret_cast<char*>(asset.indices.data()),
            static_cast<std::streamsize>(header.indexCount * sizeof(std::uint32_t)));

    if (!in) {
        Zenith::Log::Error("Failed to read complete asset data from {}", assetPath.string());
        return std::nullopt;
    }

    return asset;
}
