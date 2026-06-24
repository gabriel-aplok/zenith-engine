#include "resource/model_loader.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <sstream>
#include <string>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <rapidobj/rapidobj.hpp>

#include "render/mesh_builder.hpp"

namespace Zenith
{
    namespace
    {
        constexpr char kBakedModelMagic[4] = { 'Z', 'N', 'M', 'D' };
        constexpr std::uint32_t kBakedModelVersion = 1;

        int resolveObjIndex(int index, std::size_t count)
        {
            if (index > 0)
                return index - 1;

            if (index < 0)
                return static_cast<int>(count) + index;

            return -1;
        }

        std::optional<Vector3> readObjPosition(const rapidobj::Attributes& attributes, int index)
        {
            const int resolved = resolveObjIndex(index, attributes.positions.size() / 3);
            if (resolved < 0)
            {
                return std::nullopt;
            }

            const std::size_t base = static_cast<std::size_t>(resolved) * 3;
            if (base + 2 >= attributes.positions.size())
            {
                return std::nullopt;
            }

            return Vector3{ attributes.positions[base + 0], attributes.positions[base + 1], attributes.positions[base + 2] };
        }

        std::optional<Vector2> readObjUv(const rapidobj::Attributes& attributes, int index)
        {
            const int resolved = resolveObjIndex(index, attributes.texcoords.size() / 2);
            if (resolved < 0)
            {
                return std::nullopt;
            }

            const std::size_t base = static_cast<std::size_t>(resolved) * 2;
            if (base + 1 >= attributes.texcoords.size())
            {
                return std::nullopt;
            }

            return Vector2{ attributes.texcoords[base + 0], 1.0f - attributes.texcoords[base + 1] };
        }

        void appendMesh(ModelResource& model, std::string name, std::vector<Render::MeshVertex> vertices, std::vector<std::uint16_t> indices, std::string materialPath = {})
        {
            if (vertices.empty() || indices.empty())
            {
                return;
            }

            MeshAssetData mesh;
            mesh.name = std::move(name);
            mesh.mesh.vertices = std::move(vertices);
            mesh.mesh.indices = std::move(indices);
            mesh.mesh.bounds = MeshBuilder::computeBounds(mesh.mesh.vertices);
            mesh.materialPath = std::move(materialPath);
            model.meshes.emplace_back(std::move(mesh));
        }

        std::optional<ModelResource> loadBuiltinModel(const std::filesystem::path& path)
        {
            const std::string name = path.string();
            ModelResource model;
            model.virtualPath = name;
            model.loaded = true;

            if (name == "builtin://cube")
            {
                auto mesh = Zenith::MeshBuilder::makeCube();
                appendMesh(model, "Cube", std::move(mesh.vertices), std::move(mesh.indices));
                return model;
            }

            if (name == "builtin://pyramid")
            {
                auto mesh = Zenith::MeshBuilder::makePyramid();
                appendMesh(model, "Pyramid", std::move(mesh.vertices), std::move(mesh.indices));
                return model;
            }

            if (name == "builtin://plane")
            {
                auto mesh = Zenith::MeshBuilder::makePlane();
                appendMesh(model, "Plane", std::move(mesh.vertices), std::move(mesh.indices));
                return model;
            }

            return std::nullopt;
        }

        bool writeString(std::ofstream& stream, const std::string& value)
        {
            const std::uint32_t size = static_cast<std::uint32_t>(value.size());
            stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
            if (!value.empty())
            {
                stream.write(value.data(), static_cast<std::streamsize>(value.size()));
            }
            return static_cast<bool>(stream);
        }

        bool readString(std::ifstream& stream, std::string& value)
        {
            std::uint32_t size = 0;
            stream.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (!stream)
            {
                return false;
            }

            value.resize(size);
            if (size > 0)
            {
                stream.read(value.data(), static_cast<std::streamsize>(size));
            }
            return static_cast<bool>(stream);
        }

        bool writeMesh(std::ofstream& stream, const MeshAssetData& mesh)
        {
            if (!writeString(stream, mesh.name) || !writeString(stream, mesh.materialPath))
            {
                return false;
            }

            const std::uint32_t vertexCount = static_cast<std::uint32_t>(mesh.mesh.vertices.size());
            const std::uint32_t indexCount = static_cast<std::uint32_t>(mesh.mesh.indices.size());
            stream.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
            stream.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
            stream.write(reinterpret_cast<const char*>(&mesh.mesh.bounds.center), sizeof(Vector3));
            stream.write(reinterpret_cast<const char*>(&mesh.mesh.bounds.extents), sizeof(Vector3));

            for (const auto& vertex : mesh.mesh.vertices)
            {
                stream.write(reinterpret_cast<const char*>(&vertex.position), sizeof(Vector3));
                stream.write(reinterpret_cast<const char*>(&vertex.uv), sizeof(Vector2));
                stream.write(reinterpret_cast<const char*>(&vertex.color), sizeof(Vector4));
            }

            for (const auto index : mesh.mesh.indices)
            {
                const std::uint32_t value = index;
                stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }

            return static_cast<bool>(stream);
        }

        bool readMesh(std::ifstream& stream, MeshAssetData& mesh)
        {
            if (!readString(stream, mesh.name) || !readString(stream, mesh.materialPath))
            {
                return false;
            }

            std::uint32_t vertexCount = 0;
            std::uint32_t indexCount = 0;
            stream.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
            stream.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
            stream.read(reinterpret_cast<char*>(&mesh.mesh.bounds.center), sizeof(Vector3));
            stream.read(reinterpret_cast<char*>(&mesh.mesh.bounds.extents), sizeof(Vector3));
            if (!stream)
            {
                return false;
            }

            mesh.mesh.vertices.resize(vertexCount);
            mesh.mesh.indices.resize(indexCount);
            for (std::uint32_t i = 0; i < vertexCount; ++i)
            {
                stream.read(reinterpret_cast<char*>(&mesh.mesh.vertices[i].position), sizeof(Vector3));
                stream.read(reinterpret_cast<char*>(&mesh.mesh.vertices[i].uv), sizeof(Vector2));
                stream.read(reinterpret_cast<char*>(&mesh.mesh.vertices[i].color), sizeof(Vector4));
                if (!stream)
                {
                    return false;
                }
            }

            for (std::uint32_t i = 0; i < indexCount; ++i)
            {
                std::uint32_t value = 0;
                stream.read(reinterpret_cast<char*>(&value), sizeof(value));
                if (!stream)
                {
                    return false;
                }
                if (value > std::numeric_limits<std::uint16_t>::max())
                {
                    return false;
                }
                mesh.mesh.indices[i] = static_cast<std::uint16_t>(value);
            }

            return true;
        }

        std::optional<ModelResource> loadBakedModel(const std::filesystem::path& path)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
            {
                return std::nullopt;
            }

            char magic[4] = {};
            std::uint32_t version = 0;
            std::uint32_t meshCount = 0;
            stream.read(magic, sizeof(magic));
            stream.read(reinterpret_cast<char*>(&version), sizeof(version));
            stream.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
            if (!stream || !std::equal(std::begin(magic), std::end(magic), std::begin(kBakedModelMagic)) || version != kBakedModelVersion)
            {
                return std::nullopt;
            }

            ModelResource model;
            model.virtualPath = path.string();
            model.bakedPath = path.string();
            model.loaded = true;
            model.meshes.reserve(meshCount);

            for (std::uint32_t i = 0; i < meshCount; ++i)
            {
                MeshAssetData mesh;
                if (!readMesh(stream, mesh))
                {
                    return std::nullopt;
                }
                model.meshes.emplace_back(std::move(mesh));
            }

            return model;
        }

        std::optional<ModelResource> loadObjModel(const std::filesystem::path& path)
        {
            auto result = rapidobj::ParseFile(path, rapidobj::MaterialLibrary::SearchPath(path.parent_path(), rapidobj::Load::Optional));
            if (result.error)
            {
                return std::nullopt;
            }

            rapidobj::Triangulate(result);

            ModelResource model;
            model.sourcePath = path.string();
            model.loaded = true;

            for (const auto& shape : result.shapes)
            {
                const auto& mesh = shape.mesh;
                if (mesh.indices.empty())
                {
                    continue;
                }

                std::vector<Render::MeshVertex> vertices;
                std::vector<std::uint16_t> indices;
                vertices.reserve(mesh.indices.size());
                indices.reserve(mesh.indices.size());

                std::string materialPath;
                if (!mesh.material_ids.empty() && !result.materials.empty())
                {
                    const int materialId = mesh.material_ids.front();
                    if (materialId >= 0 && materialId < static_cast<int>(result.materials.size()))
                    {
                        const auto& material = result.materials[static_cast<std::size_t>(materialId)];
                        if (!material.diffuse_texname.empty())
                        {
                            materialPath = (path.parent_path() / material.diffuse_texname).string();
                        }
                    }
                }

                for (const auto& objIndex : mesh.indices)
                {
                    const auto position = readObjPosition(result.attributes, objIndex.position_index);
                    if (!position)
                    {
                        continue;
                    }

                    Vector2 uv{ (position->x + 1.0f) * 0.5f, (position->z + 1.0f) * 0.5f };
                    if (const auto texcoord = readObjUv(result.attributes, objIndex.texcoord_index))
                    {
                        uv = *texcoord;
                    }

                    vertices.push_back(Render::MeshVertex{ *position, uv, Vector4{1.0f} });
                    const auto nextIndex = static_cast<std::uint32_t>(vertices.size() - 1);
                    if (nextIndex > std::numeric_limits<std::uint16_t>::max())
                    {
                        return std::nullopt;
                    }
                    indices.push_back(static_cast<std::uint16_t>(nextIndex));
                }

                appendMesh(model, shape.name.empty() ? path.stem().string() : shape.name, std::move(vertices), std::move(indices), std::move(materialPath));
            }

            return model;
        }

        template <typename T>
        std::vector<T> copyAccessor(const fastgltf::Asset& asset, const fastgltf::Accessor& accessor)
        {
            std::vector<T> values;
            values.reserve(accessor.count);
            for (const auto& value : fastgltf::iterateAccessor<T>(asset, accessor))
            {
                values.push_back(value);
            }
            return values;
        }

        std::optional<ModelResource> loadGltfModel(const std::filesystem::path& path)
        {
            auto buffer = fastgltf::GltfDataBuffer::FromPath(path);
            if (!buffer)
            {
                return std::nullopt;
            }

            fastgltf::Parser parser(fastgltf::Extensions::None);
            auto assetResult = parser.loadGltf(buffer.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers | fastgltf::Options::LoadExternalImages | fastgltf::Options::DecomposeNodeMatrices | fastgltf::Options::GenerateMeshIndices, fastgltf::Category::OnlyRenderable);
            if (!assetResult)
            {
                return std::nullopt;
            }

            fastgltf::Asset asset = std::move(assetResult.get());

            ModelResource model;
            model.sourcePath = path.string();
            model.loaded = true;

            for (const auto& mesh : asset.meshes)
            {
                for (const auto& primitive : mesh.primitives)
                {
                    if (primitive.type != fastgltf::PrimitiveType::Triangles)
                    {
                        continue;
                    }

                    const auto positionIt = primitive.findAttribute("POSITION");
                    if (positionIt == primitive.attributes.end())
                    {
                        continue;
                    }

                    const auto& positionAccessor = asset.accessors[positionIt->accessorIndex];
                    const auto positions = copyAccessor<fastgltf::math::fvec3>(asset, positionAccessor);

                    std::vector<fastgltf::math::fvec2> texcoords;
                    if (const auto texcoordIt = primitive.findAttribute("TEXCOORD_0"); texcoordIt != primitive.attributes.end())
                    {
                        texcoords = copyAccessor<fastgltf::math::fvec2>(asset, asset.accessors[texcoordIt->accessorIndex]);
                    }

                    std::vector<std::uint32_t> sourceIndices;
                    if (primitive.indicesAccessor.has_value())
                    {
                        sourceIndices = copyAccessor<std::uint32_t>(asset, asset.accessors[*primitive.indicesAccessor]);
                    }
                    else
                    {
                        sourceIndices.resize(positions.size());
                        for (std::uint32_t i = 0; i < sourceIndices.size(); ++i)
                        {
                            sourceIndices[i] = i;
                        }
                    }

                    std::vector<Render::MeshVertex> vertices;
                    std::vector<std::uint16_t> indices;
                    vertices.reserve(sourceIndices.size());
                    indices.reserve(sourceIndices.size());

                    for (const auto indexValue : sourceIndices)
                    {
                        if (indexValue >= positions.size())
                        {
                            continue;
                        }

                        const auto position = positions[indexValue];
                        Vector3 vertexPosition{ position.x(), position.y(), position.z() };
                        Vector2 uv{ (vertexPosition.x + 1.0f) * 0.5f, (vertexPosition.z + 1.0f) * 0.5f };
                        if (indexValue < texcoords.size())
                        {
                            const auto texcoord = texcoords[indexValue];
                            uv = Vector2{ texcoord.x(), texcoord.y() };
                        }

                        vertices.push_back(Render::MeshVertex{ vertexPosition, uv, Vector4{1.0f} });
                        const auto nextIndex = static_cast<std::uint32_t>(vertices.size() - 1);
                        if (nextIndex > std::numeric_limits<std::uint16_t>::max())
                        {
                            return std::nullopt;
                        }
                        indices.push_back(static_cast<std::uint16_t>(nextIndex));
                    }

                    std::string materialPath;
                    if (primitive.materialIndex.has_value() && *primitive.materialIndex < asset.materials.size())
                    {
                        const auto& material = asset.materials[*primitive.materialIndex];
                        if (!material.name.empty())
                        {
                            materialPath = material.name;
                        }
                    }

                    appendMesh(model, mesh.name.empty() ? path.stem().string() : std::string(mesh.name), std::move(vertices), std::move(indices), std::move(materialPath));
                }
            }

            return model.meshes.empty() ? std::nullopt : std::optional<ModelResource>{ std::move(model) };
        }

        void writeBakedModel(const std::filesystem::path& path, const ModelResource& model)
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                return;
            }

            stream.write(kBakedModelMagic, sizeof(kBakedModelMagic));
            stream.write(reinterpret_cast<const char*>(&kBakedModelVersion), sizeof(kBakedModelVersion));
            const std::uint32_t meshCount = static_cast<std::uint32_t>(model.meshes.size());
            stream.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
            for (const auto& mesh : model.meshes)
            {
                if (!writeMesh(stream, mesh))
                {
                    return;
                }
            }
        }
    } // namespace

    void ModelLoaderRegistry::registerLoader(const std::string& extension, Loader loader)
    {
        m_loaders[normalizeExtension(extension)] = std::move(loader);
    }

    std::optional<ModelResource> ModelLoaderRegistry::load(const std::filesystem::path& path) const
    {
        if (const std::string name = path.string(); name == "builtin://cube" || name == "builtin://pyramid" || name == "builtin://plane")
        {
            return loadBuiltinModel(path);
        }

        const std::string ext = normalizeExtension(path.extension().string());
        const auto it = m_loaders.find(ext);
        if (it == m_loaders.end() || !it->second)
        {
            return std::nullopt;
        }

        return it->second(path);
    }

    void ModelLoaderRegistry::clear()
    {
        m_loaders.clear();
    }

    std::string ModelLoaderRegistry::normalizeExtension(std::string extension)
    {
        std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c)
            { return static_cast<char>(std::tolower(c)); });
        if (!extension.empty() && extension.front() != '.')
        {
            extension.insert(extension.begin(), '.');
        }
        return extension;
    }

    ModelLoaderRegistry& defaultModelLoaderRegistry()
    {
        static ModelLoaderRegistry registry;
        static bool initialized = false;
        if (!initialized)
        {
            registerStandardModelLoaders(registry);
            initialized = true;
        }
        return registry;
    }

    void registerStandardModelLoaders(ModelLoaderRegistry& registry)
    {
        registry.registerLoader(".obj", loadObjModel);
        registry.registerLoader(".gltf", loadGltfModel);
        registry.registerLoader(".glb", loadGltfModel);
        registry.registerLoader(".modelbin", loadBakedModel);
        registry.registerLoader(".zenith", loadBakedModel);
    }
} // namespace Zenith
