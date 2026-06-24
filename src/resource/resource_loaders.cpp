#include "resource/resource_loaders.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include <lodepng.h>

#include "render/mesh_builder.hpp"
#include "resource/baked_mesh_asset.hpp"
#include "resource/binary_source.hpp"
#include "resource/image_source.hpp"
#include "resource/mesh_source.hpp"
#include "resource/obj_mesh_loader.hpp"
#include "resource/resource_manager.hpp"
#include "resource/texture_asset.hpp"
#include "resource/text_source.hpp"

namespace Zenith
{
    namespace
    {
        std::shared_ptr<MeshSource> loadBuiltinMeshSource(const std::string &path)
        {
            const Render::MeshData *mesh = nullptr;
            Render::MeshData ownedMesh;

            if (path == "builtin://cube")
            {
                ownedMesh = MeshBuilder::makeCube();
                mesh = &ownedMesh;
            }
            else if (path == "builtin://pyramid")
            {
                ownedMesh = MeshBuilder::makePyramid();
                mesh = &ownedMesh;
            }
            else if (path == "builtin://plane")
            {
                ownedMesh = MeshBuilder::makePlane();
                mesh = &ownedMesh;
            }

            if (!mesh)
            {
                return nullptr;
            }

            auto source = std::make_shared<MeshSource>();
            source->vertices.reserve(mesh->vertices.size());
            for (const auto &vertex : mesh->vertices)
            {
                source->vertices.push_back(MeshSourceVertex{vertex.position, vertex.uv, vertex.color});
            }
            source->indices = mesh->indices;
            source->bounds = {mesh->bounds.center, mesh->bounds.extents};
            return source;
        }

        std::shared_ptr<MeshSource> loadMeshSourceFile(const std::filesystem::path &path)
        {
            if (auto loaded = loadObjMesh(path))
            {
                auto source = std::make_shared<MeshSource>();
                source->vertices.reserve(loaded->vertices.size());
                for (const auto &vertex : loaded->vertices)
                {
                    source->vertices.push_back(MeshSourceVertex{vertex.position, vertex.uv, vertex.color});
                }
                source->indices = loaded->indices;
                source->bounds = {loaded->bounds.center, loaded->bounds.extents};
                return source;
            }

            return nullptr;
        }

        std::shared_ptr<BakedMeshAsset> loadBakedMeshAssetFromSource(const std::string &path)
        {
            auto source = loadBuiltinMeshSource(path);
            if (!source)
            {
                const std::filesystem::path filePath = path;
                if (filePath.extension() == ".obj")
                {
                    source = loadMeshSourceFile(filePath);
                }
            }

            if (!source)
            {
                return nullptr;
            }

            auto asset = std::make_shared<BakedMeshAsset>();
            asset->sourcePath = path;
            asset->mesh.vertices.reserve(source->vertices.size());
            for (const auto &vertex : source->vertices)
            {
                asset->mesh.vertices.push_back(Render::MeshVertex{vertex.position, vertex.uv, vertex.color});
            }
            asset->mesh.indices = source->indices;
            asset->mesh.bounds = {source->bounds.center, source->bounds.extents};
            return asset;
        }

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

        std::shared_ptr<ImageSource> loadImageSourceFile(const std::filesystem::path &path)
        {
            std::vector<unsigned char> encoded;
            unsigned width = 0;
            unsigned height = 0;

            const unsigned decodeError = lodepng::decode(encoded, width, height, path.string());
            if (decodeError != 0)
            {
                return nullptr;
            }

            ImageSourceData data;
            data.width = width;
            data.height = height;
            data.pixels.assign(encoded.begin(), encoded.end());
            data.format = path.extension().string();
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
        resources.registerLoader<MeshSource>([](const std::string &path) -> std::shared_ptr<MeshSource>
                                             {
            if (auto builtin = loadBuiltinMeshSource(path))
            {
                return builtin;
            }

            const std::filesystem::path filePath = path;
            if (filePath.extension() == ".obj")
            {
                return loadMeshSourceFile(filePath);
            }

            return nullptr; });

        resources.registerLoader<Render::MeshData>([](const std::string &path) -> std::shared_ptr<Render::MeshData>
                                                    {
            if (auto builtin = loadBuiltinMeshSource(path))
            {
                Render::MeshData mesh;
                mesh.vertices.reserve(builtin->vertices.size());
                for (const auto &vertex : builtin->vertices)
                {
                    mesh.vertices.push_back(Render::MeshVertex{vertex.position, vertex.uv, vertex.color});
                }
                mesh.indices = builtin->indices;
                mesh.bounds = {builtin->bounds.center, builtin->bounds.extents};
                return std::make_shared<Render::MeshData>(std::move(mesh));
            }

            const std::filesystem::path filePath = path;
            if (filePath.extension() == ".obj")
            {
                if (auto source = loadMeshSourceFile(filePath))
                {
                    Render::MeshData mesh;
                    mesh.vertices.reserve(source->vertices.size());
                    for (const auto &vertex : source->vertices)
                    {
                        mesh.vertices.push_back(Render::MeshVertex{vertex.position, vertex.uv, vertex.color});
                    }
                    mesh.indices = source->indices;
                    mesh.bounds = {source->bounds.center, source->bounds.extents};
                    return std::make_shared<Render::MeshData>(std::move(mesh));
                }
            }

            return nullptr; });

        resources.registerLoader<BakedMeshAsset>([](const std::string &path) -> std::shared_ptr<BakedMeshAsset>
                                                 { return loadBakedMeshAssetFromSource(path); });

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
            {
                return nullptr;
            }

            const auto ext = filePath.extension().string();
            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".tga" && ext != ".gif" && ext != ".webp")
            {
                return nullptr;
            }

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
