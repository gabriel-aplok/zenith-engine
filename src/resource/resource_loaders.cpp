#include "resource/resource_loaders.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include "render/mesh_builder.hpp"
#include "resource/binary_source.hpp"
#include "resource/obj_mesh_loader.hpp"
#include "resource/resource_manager.hpp"
#include "resource/text_source.hpp"

namespace Zenith
{
    namespace
    {
        std::shared_ptr<TextSource> loadTextAssetFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                return nullptr;
            }

            auto asset = std::make_shared<TextSource>(std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()}, path.string());
            return asset;
        }

        std::shared_ptr<BinarySource> loadBinaryAssetFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                return nullptr;
            }

            std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            return std::make_shared<BinarySource>(std::move(bytes), path.string());
        }
    } // namespace

    void registerStandardResourceLoaders(ResourceManager &resources)
    {
        resources.registerLoader<Render::MeshData>([](const std::string &path) -> std::shared_ptr<Render::MeshData>
                                                   {
            if (path == "builtin://cube")
            {
                return std::make_shared<Render::MeshData>(MeshBuilder::makeCube());
            }

            if (path == "builtin://pyramid")
            {
                return std::make_shared<Render::MeshData>(MeshBuilder::makePyramid());
            }

            if (path == "builtin://plane")
            {
                return std::make_shared<Render::MeshData>(MeshBuilder::makePlane());
            }

            const std::filesystem::path filePath = path;
            if (filePath.extension() == ".obj")
            {
                if (auto loaded = loadObjMesh(filePath))
                {
                    return std::make_shared<Render::MeshData>(std::move(*loaded));
                }
            }

            return nullptr; });

        resources.registerLoader<TextSource>([](const std::string &path) -> std::shared_ptr<TextSource>
                                            {
            const std::filesystem::path filePath = path;
            return loadTextAssetFile(filePath); });

        resources.registerLoader<BinarySource>([](const std::string &path) -> std::shared_ptr<BinarySource>
                                                {
            const std::filesystem::path filePath = path;
            return loadBinaryAssetFile(filePath); });
    }
} // namespace Zenith
