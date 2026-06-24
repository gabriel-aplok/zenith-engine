#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "math/math.hpp"
#include "render/mesh.hpp"

namespace Zenith
{
    enum class ResourceKind : std::uint32_t
    {
        Unknown = 0,
        Image,
        Texture,
        Mesh,
        Model,
        Shader,
        Material,
    };

    struct ImageData
    {
        std::uint32_t width = 0;
        std::uint32_t height = 0;
        std::vector<std::uint8_t> pixels;
        std::string format = "rgba8";
    };

    using ImageSourceData = ImageData;

    struct TextureImportSettings
    {
        std::string compressionMode = "none";
        bool generateMipmaps = true;
        bool srgb = true;
        std::string wrapMode = "repeat";
        std::string filterMode = "linear";
        std::uint32_t maxTextureSize = 0;
        bool normalMap = false;
        std::uint32_t importerVersion = 1;
    };

    struct ModelImportSettings
    {
        bool triangulate = true;
        bool loadMaterials = true;
        bool generateNormals = false;
        std::uint32_t importerVersion = 1;
    };

    struct ShaderImportSettings
    {
        std::string stage = "unknown";
        std::uint32_t importerVersion = 1;
    };

    struct MaterialImportSettings
    {
        std::uint32_t importerVersion = 1;
    };

    struct MeshAssetData
    {
        std::string name;
        Render::MeshData mesh;
        std::string materialPath;
        Render::MeshHandle handle{};
    };

    struct Resource
    {
        explicit Resource(ResourceKind resourceKind = ResourceKind::Unknown) noexcept
            : kind(resourceKind)
        {
        }

        virtual ~Resource() = default;

        ResourceKind kind = ResourceKind::Unknown;
        std::string stableId;
        std::string virtualPath;
        std::string sourcePath;
        std::string bakedPath;
        std::string metadataPath;
        std::uint64_t assetHash = 0;
        bool loaded = false;
        std::vector<std::string> dependencies;
    };

    struct ImageResource final : public Resource
    {
        ImageResource() : Resource(ResourceKind::Image) {}
        ImageData image;
    };

    struct TextureResource final : public Resource
    {
        TextureResource() : Resource(ResourceKind::Texture) {}
        ImageData image;
        TextureImportSettings importSettings{};
        Render::TextureHandle handle{};
    };

    struct MeshResource final : public Resource
    {
        MeshResource() : Resource(ResourceKind::Mesh) {}
        ModelImportSettings importSettings{};
        MeshAssetData mesh;
    };

    struct ModelResource final : public Resource
    {
        ModelResource() : Resource(ResourceKind::Model) {}
        ModelImportSettings importSettings{};
        std::vector<MeshAssetData> meshes;
    };

    struct ShaderResource final : public Resource
    {
        ShaderResource() : Resource(ResourceKind::Shader) {}
        ShaderImportSettings importSettings{};
        std::string source;
        std::vector<std::uint8_t> bytecode;
    };

    struct MaterialResource final : public Resource
    {
        MaterialResource() : Resource(ResourceKind::Material) {}
        MaterialImportSettings importSettings{};
        Render::MaterialState state{};
        std::string source;
        std::string shaderPath;
        std::vector<std::string> texturePaths;
    };

    template <typename T>
    struct ResourceTraits;

    template <>
    struct ResourceTraits<ImageResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Image;
    };

    template <>
    struct ResourceTraits<TextureResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Texture;
    };

    template <>
    struct ResourceTraits<MeshResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Mesh;
    };

    template <>
    struct ResourceTraits<ModelResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Model;
    };

    template <>
    struct ResourceTraits<ShaderResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Shader;
    };

    template <>
    struct ResourceTraits<MaterialResource>
    {
        static constexpr ResourceKind kind = ResourceKind::Material;
    };

} // namespace Zenith
