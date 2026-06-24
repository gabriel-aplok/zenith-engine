#include "resource/resource_loaders.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "resource/baked_mesh_asset.hpp"
#include "resource/binary_source.hpp"
#include "resource/image_source.hpp"
#include "resource/model_loader.hpp"
#include "resource/resource_manager.hpp"
#include "resource/texture_asset.hpp"
#include "resource/text_source.hpp"

namespace Zenith
{
    namespace
    {
        std::shared_ptr<Render::MeshData> loadMeshDataFromModelPath(const std::string &path)
        {
            const std::filesystem::path filePath = path;
            if (auto mesh = defaultModelLoaderRegistry().load(filePath))
                return std::make_shared<Render::MeshData>(std::move(*mesh));

            return nullptr;
        }

        std::shared_ptr<BakedMeshAsset> loadBakedMeshAssetFromSource(const std::string &path)
        {
            auto mesh = loadMeshDataFromModelPath(path);
            if (!mesh)
                return nullptr;

            auto asset = std::make_shared<BakedMeshAsset>();
            asset->sourcePath = path;
            asset->mesh = std::move(*mesh);
            return asset;
        }

        std::shared_ptr<TextSource> loadTextAssetFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
                return nullptr;

            auto asset = std::make_shared<TextSource>(std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()}, path.string());
            return asset;
        }

        std::shared_ptr<BinarySource> loadBinaryAssetFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
                return nullptr;

            std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            return std::make_shared<BinarySource>(std::move(bytes), path.string());
        }

        std::shared_ptr<ImageSource> loadImageSourceFile(const std::filesystem::path &path)
        {
            int width = 0;
            int height = 0;
            int channels = 0;

            stbi_uc *pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels)
                return nullptr;

            ImageSourceData data;
            data.width = static_cast<std::uint32_t>(width);
            data.height = static_cast<std::uint32_t>(height);
            data.pixels.assign(pixels, pixels + (static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4));
            data.format = "rgba8";
            stbi_image_free(pixels);
            return std::make_shared<ImageSource>(std::move(data), path.string());
        }

        std::shared_ptr<TextureAsset> loadTextureAssetFile(const std::filesystem::path &path)
        {
            if (auto image = loadImageSourceFile(path))
            {
                TextureAsset asset;
                asset.source = *image;
                asset.sourcePath = path.string();
                return std::make_shared<TextureAsset>(std::move(asset));
            }

            return nullptr;
        }
    } // namespace

    void registerStandardResourceLoaders(ResourceManager &resources)
    {
        resources.registerLoader<BakedMeshAsset>([](const std::string &path) -> std::shared_ptr<BakedMeshAsset>
                                                 { return loadBakedMeshAssetFromSource(path); });

        resources.registerLoader<Render::MeshData>([](const std::string &path) -> std::shared_ptr<Render::MeshData>
                                                   { return loadMeshDataFromModelPath(path); });

        resources.registerLoader<TextSource>([](const std::string &path) -> std::shared_ptr<TextSource>
                                             {
            const std::filesystem::path filePath = path;
            return loadTextAssetFile(filePath); });

        resources.registerLoader<BinarySource>([](const std::string &path) -> std::shared_ptr<BinarySource>
                                               {
            const std::filesystem::path filePath = path;
            return loadBinaryAssetFile(filePath); });

        resources.registerLoader<ImageSource>([](const std::string &path) -> std::shared_ptr<ImageSource>
                                              {
            const std::filesystem::path filePath = path;
            if (!filePath.has_extension()) 
                return nullptr; 

            const auto ext = filePath.extension().string();
            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".tga" && ext != ".gif" && ext != ".webp")
                return nullptr;

            return loadImageSourceFile(filePath); });

        resources.registerLoader<TextureAsset>([](const std::string &path) -> std::shared_ptr<TextureAsset>
                                               {
            const std::filesystem::path filePath = path;
            if (!filePath.has_extension())
            {
                return nullptr;
            }

            const auto ext = filePath.extension().string();
            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".tga" && ext != ".gif" && ext != ".webp")
            {
                return nullptr;
            }

            return loadTextureAssetFile(filePath); });
    }
} // namespace Zenith
