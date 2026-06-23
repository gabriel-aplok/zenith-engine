#include "resource/resource_loaders.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>

#include "render/mesh_builder.hpp"
#include "resource/obj_mesh_loader.hpp"
#include "resource/resource_manager.hpp"
#include "resource/text_asset.hpp"

namespace Zenith
{
    namespace
    {
        std::shared_ptr<TextAsset> loadTextAssetFile(const std::filesystem::path &path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                return nullptr;
            }

            auto asset = std::make_shared<TextAsset>();
            asset->text.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            return asset;
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

        resources.registerLoader<TextAsset>([](const std::string &path) -> std::shared_ptr<TextAsset>
                                            {
            const std::filesystem::path filePath = path;
            return loadTextAssetFile(filePath); });
    }
} // namespace Zenith
