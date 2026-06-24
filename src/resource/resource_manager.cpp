#include "resource/resource_manager.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <optional>
#include <system_error>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include "render/mesh_builder.hpp"

namespace Zenith
{
    namespace
    {
        constexpr char kImageMagic[4] = { 'Z', 'I', 'M', 'G' };
        constexpr char kTextMagic[4] = { 'Z', 'T', 'X', 'T' };
        constexpr std::uint32_t kImageVersion = 1;
        constexpr std::uint32_t kTextVersion = 1;

        constexpr std::array<const char*, 6> kImageExtensions = { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".webp" };

        std::uint64_t fnv1a64(const std::uint8_t* data, std::size_t size)
        {
            std::uint64_t hash = 1469598103934665603ull;
            for (std::size_t i = 0; i < size; ++i)
            {
                hash ^= data[i];
                hash *= 1099511628211ull;
            }
            return hash;
        }

        std::uint64_t hashFile(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                return 0;
            }

            std::vector<std::uint8_t> bytes{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
            return bytes.empty() ? 0 : fnv1a64(bytes.data(), bytes.size());
        }

        std::string readFileText(const std::filesystem::path& path)
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
            {
                return {};
            }

            return std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
        }

        std::optional<ImageData> decodeImageFile(const std::filesystem::path& path)
        {
            int width = 0;
            int height = 0;
            int channels = 0;
            stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels)
            {
                return std::nullopt;
            }

            ImageData image;
            image.width = static_cast<std::uint32_t>(width);
            image.height = static_cast<std::uint32_t>(height);
            image.pixels.assign(pixels, pixels + (static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4));
            image.format = "rgba8";
            stbi_image_free(pixels);
            return image;
        }

        ImageData makeCheckerImage()
        {
            ImageData image;
            image.width = 2;
            image.height = 2;
            image.format = "rgba8";
            image.pixels = {
                255, 255, 255, 255,
                32, 32, 32, 255,
                32, 32, 32, 255,
                255, 255, 255, 255,
            };
            return image;
        }

        std::string serializeTextureSettings(const TextureImportSettings& settings)
        {
            rapidjson::Document document;
            document.SetObject();
            auto& allocator = document.GetAllocator();
            document.AddMember("compressionMode", rapidjson::Value(settings.compressionMode.c_str(), allocator), allocator);
            document.AddMember("generateMipmaps", settings.generateMipmaps, allocator);
            document.AddMember("srgb", settings.srgb, allocator);
            document.AddMember("wrapMode", rapidjson::Value(settings.wrapMode.c_str(), allocator), allocator);
            document.AddMember("filterMode", rapidjson::Value(settings.filterMode.c_str(), allocator), allocator);
            document.AddMember("maxTextureSize", settings.maxTextureSize, allocator);
            document.AddMember("normalMap", settings.normalMap, allocator);
            document.AddMember("importerVersion", settings.importerVersion, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            return buffer.GetString();
        }

        std::string serializeModelSettings(const ModelImportSettings& settings)
        {
            rapidjson::Document document;
            document.SetObject();
            auto& allocator = document.GetAllocator();
            document.AddMember("triangulate", settings.triangulate, allocator);
            document.AddMember("loadMaterials", settings.loadMaterials, allocator);
            document.AddMember("generateNormals", settings.generateNormals, allocator);
            document.AddMember("importerVersion", settings.importerVersion, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            return buffer.GetString();
        }

        std::string serializeShaderSettings(const ShaderImportSettings& settings)
        {
            rapidjson::Document document;
            document.SetObject();
            auto& allocator = document.GetAllocator();
            document.AddMember("stage", rapidjson::Value(settings.stage.c_str(), allocator), allocator);
            document.AddMember("importerVersion", settings.importerVersion, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            return buffer.GetString();
        }

        std::string serializeMaterialSettings(const MaterialImportSettings& settings)
        {
            rapidjson::Document document;
            document.SetObject();
            auto& allocator = document.GetAllocator();
            document.AddMember("importerVersion", settings.importerVersion, allocator);

            rapidjson::StringBuffer buffer;
            rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
            document.Accept(writer);
            return buffer.GetString();
        }

        bool writeBinaryBlob(const std::filesystem::path& path, const char* magic, std::uint32_t version, const std::vector<std::uint8_t>& bytes)
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                return false;
            }

            stream.write(magic, 4);
            stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
            const std::uint64_t size = static_cast<std::uint64_t>(bytes.size());
            stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
            if (!bytes.empty())
            {
                stream.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
            }
            return static_cast<bool>(stream);
        }

        bool readBinaryBlob(const std::filesystem::path& path, const char* magic, std::uint32_t version, std::vector<std::uint8_t>& bytes)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
            {
                return false;
            }

            char fileMagic[4] = {};
            std::uint32_t fileVersion = 0;
            std::uint64_t size = 0;
            stream.read(fileMagic, 4);
            stream.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            stream.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (!stream || !std::equal(std::begin(fileMagic), std::end(fileMagic), magic) || fileVersion != version)
            {
                return false;
            }

            bytes.resize(static_cast<std::size_t>(size));
            if (size > 0)
            {
                stream.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(size));
            }
            return static_cast<bool>(stream);
        }

        bool writeTextBlob(const std::filesystem::path& path, const char* magic, std::uint32_t version, const std::string& text)
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                return false;
            }

            stream.write(magic, 4);
            stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
            const std::uint64_t size = static_cast<std::uint64_t>(text.size());
            stream.write(reinterpret_cast<const char*>(&size), sizeof(size));
            if (!text.empty())
            {
                stream.write(text.data(), static_cast<std::streamsize>(text.size()));
            }
            return static_cast<bool>(stream);
        }

        bool readTextBlob(const std::filesystem::path& path, const char* magic, std::uint32_t version, std::string& text)
        {
            std::ifstream stream(path, std::ios::binary);
            if (!stream)
            {
                return false;
            }

            char fileMagic[4] = {};
            std::uint32_t fileVersion = 0;
            std::uint64_t size = 0;
            stream.read(fileMagic, 4);
            stream.read(reinterpret_cast<char*>(&fileVersion), sizeof(fileVersion));
            stream.read(reinterpret_cast<char*>(&size), sizeof(size));
            if (!stream || !std::equal(std::begin(fileMagic), std::end(fileMagic), magic) || fileVersion != version)
            {
                return false;
            }

            text.resize(static_cast<std::size_t>(size));
            if (size > 0)
            {
                stream.read(text.data(), static_cast<std::streamsize>(size));
            }
            return static_cast<bool>(stream);
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

        bool writeBakedModelFile(const std::filesystem::path& path, const ModelResource& model)
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);

            std::ofstream stream(path, std::ios::binary | std::ios::trunc);
            if (!stream)
            {
                return false;
            }

            constexpr char magic[4] = { 'Z', 'N', 'M', 'D' };
            constexpr std::uint32_t version = 1;
            stream.write(magic, sizeof(magic));
            stream.write(reinterpret_cast<const char*>(&version), sizeof(version));
            const std::uint32_t meshCount = static_cast<std::uint32_t>(model.meshes.size());
            stream.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
            for (const auto& mesh : model.meshes)
            {
                if (!writeMesh(stream, mesh))
                {
                    return false;
                }
            }
            return static_cast<bool>(stream);
        }

        std::vector<std::uint8_t> buildImagePayload(const ImageData& image)
        {
            std::vector<std::uint8_t> payload;
            auto appendBytes = [&payload](const auto& value)
                {
                    const auto oldSize = payload.size();
                    payload.resize(oldSize + sizeof(value));
                    std::memcpy(payload.data() + oldSize, &value, sizeof(value));
                };
            auto appendString = [&payload, &appendBytes](const std::string& value)
                {
                    const std::uint32_t size = static_cast<std::uint32_t>(value.size());
                    appendBytes(size);
                    const auto oldSize = payload.size();
                    payload.resize(oldSize + value.size());
                    std::memcpy(payload.data() + oldSize, value.data(), value.size());
                };

            appendBytes(image.width);
            appendBytes(image.height);
            appendString(image.format);
            const std::uint32_t pixelCount = static_cast<std::uint32_t>(image.pixels.size());
            appendBytes(pixelCount);
            const auto oldSize = payload.size();
            payload.resize(oldSize + image.pixels.size());
            std::memcpy(payload.data() + oldSize, image.pixels.data(), image.pixels.size());
            return payload;
        }

        std::optional<ImageData> parseImagePayload(const std::vector<std::uint8_t>& payload)
        {
            if (payload.size() < sizeof(std::uint32_t) * 3)
            {
                return std::nullopt;
            }

            std::size_t offset = 0;
            auto readU32 = [&payload, &offset]() -> std::uint32_t
                {
                    std::uint32_t value = 0;
                    std::memcpy(&value, payload.data() + offset, sizeof(value));
                    offset += sizeof(value);
                    return value;
                };
            auto readString = [&payload, &offset, &readU32]() -> std::string
                {
                    const std::uint32_t size = readU32();
                    if (offset + size > payload.size())
                    {
                        return {};
                    }
                    std::string text(reinterpret_cast<const char*>(payload.data() + offset), size);
                    offset += size;
                    return text;
                };

            ImageData image;
            image.width = readU32();
            image.height = readU32();
            image.format = readString();
            const std::uint32_t pixelCount = readU32();
            const std::size_t pixelBytes = static_cast<std::size_t>(pixelCount);
            if (offset + pixelBytes > payload.size())
            {
                return std::nullopt;
            }

            image.pixels.assign(payload.begin() + static_cast<std::ptrdiff_t>(offset), payload.begin() + static_cast<std::ptrdiff_t>(offset + pixelBytes));
            return image;
        }

        std::optional<ImportMetadata> loadMetadataIfFresh(const std::filesystem::path& metadataPath, std::uint64_t sourceHash, std::string_view settingsJson, std::uint32_t importerVersion, const std::filesystem::path& expectedBakedPath)
        {
            ImportMetadata metadata;
            if (!readImportMetadata(metadataPath, metadata))
            {
                return std::nullopt;
            }

            if (metadata.sourceHash != sourceHash || metadata.importerVersion != importerVersion || metadata.settingsJson != settingsJson || metadata.bakedPath != expectedBakedPath.string())
            {
                return std::nullopt;
            }

            if (!std::filesystem::exists(expectedBakedPath))
            {
                return std::nullopt;
            }

            return metadata;
        }

        void stampMetadata(ImportMetadata& metadata, const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, std::uint64_t sourceHash, std::string_view importerName, std::uint32_t importerVersion, std::string settingsJson, std::vector<std::string> dependencies)
        {
            metadata.virtualPath = virtualPath.string();
            metadata.sourcePath = sourcePath.string();
            metadata.bakedPath = bakedPath.string();
            metadata.importerName = std::string(importerName);
            metadata.importerVersion = importerVersion;
            metadata.sourceHash = sourceHash;
            metadata.settingsJson = std::move(settingsJson);
            metadata.dependencies = std::move(dependencies);
            metadata.importedAtUtc = static_cast<std::int64_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        }

        void applyResourceMetadata(Resource& resource, const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash)
        {
            resource.stableId = virtualPath.string();
            resource.virtualPath = virtualPath.string();
            resource.sourcePath = sourcePath.string();
            resource.bakedPath = bakedPath.string();
            resource.metadataPath = metadataPath.string();
            resource.assetHash = sourceHash;
            resource.loaded = true;
        }

        std::string cacheKeyFor(const ResourcePath& path)
        {
            return path.string();
        }

        bool isImageExtension(const std::filesystem::path& path)
        {
            const auto ext = path.extension().string();
            return std::find_if(kImageExtensions.begin(), kImageExtensions.end(), [&ext](const char* candidate)
                { return ext == candidate; }) != kImageExtensions.end();
        }

        std::shared_ptr<ImageResource> makeImageResource(const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash, ImageData image)
        {
            auto resource = std::make_shared<ImageResource>();
            applyResourceMetadata(*resource, virtualPath, sourcePath, bakedPath, metadataPath, sourceHash);
            resource->image = std::move(image);
            resource->dependencies = {};
            return resource;
        }

        std::shared_ptr<TextureResource> makeTextureResource(const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash, ImageData image, TextureImportSettings settings)
        {
            auto resource = std::make_shared<TextureResource>();
            applyResourceMetadata(*resource, virtualPath, sourcePath, bakedPath, metadataPath, sourceHash);
            resource->image = std::move(image);
            resource->importSettings = std::move(settings);
            return resource;
        }

        std::shared_ptr<ShaderResource> makeShaderResource(const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash, std::string source, ShaderImportSettings settings)
        {
            auto resource = std::make_shared<ShaderResource>();
            applyResourceMetadata(*resource, virtualPath, sourcePath, bakedPath, metadataPath, sourceHash);
            resource->source = std::move(source);
            resource->importSettings = std::move(settings);
            return resource;
        }

        std::shared_ptr<MaterialResource> makeMaterialResource(const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash, std::string source, MaterialImportSettings settings)
        {
            auto resource = std::make_shared<MaterialResource>();
            applyResourceMetadata(*resource, virtualPath, sourcePath, bakedPath, metadataPath, sourceHash);
            resource->source = std::move(source);
            resource->importSettings = std::move(settings);
            return resource;
        }

        std::shared_ptr<MeshResource> makeMeshResource(const ResourcePath& virtualPath, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath, std::uint64_t sourceHash, MeshAssetData mesh)
        {
            auto resource = std::make_shared<MeshResource>();
            applyResourceMetadata(*resource, virtualPath, sourcePath, bakedPath, metadataPath, sourceHash);
            resource->mesh = std::move(mesh);
            return resource;
        }

    } // namespace

    ResourceManager::ResourceManager()
    {
        setProjectRoot(std::filesystem::current_path());
        setUserRoot(std::filesystem::current_path() / "user");
        setEditorRoot(std::filesystem::current_path() / "editor");
        m_vfs.setCacheRoot(std::filesystem::current_path() / ".imports");
        m_vfs.setBuiltinRoot(std::filesystem::current_path() / "resources");
        registerStandardModelLoaders(m_modelLoaders);
    }

    void ResourceManager::setProjectRoot(std::filesystem::path root)
    {
        m_vfs.setProjectRoot(std::move(root));
    }

    void ResourceManager::setUserRoot(std::filesystem::path root)
    {
        m_vfs.setUserRoot(std::move(root));
    }

    void ResourceManager::setEditorRoot(std::filesystem::path root)
    {
        m_vfs.setEditorRoot(std::move(root));
    }

    void ResourceManager::clear()
    {
        m_cache.clear();
    }

    std::string ResourceManager::cacheKey(ResourceKind, const ResourcePath& path) const
    {
        return cacheKeyFor(path);
    }

    ResourceManager::CacheBucket& ResourceManager::cacheBucket(std::type_index typeIndex)
    {
        return m_cache[typeIndex];
    }

    const ResourceManager::CacheBucket& ResourceManager::cacheBucket(std::type_index typeIndex) const
    {
        const auto it = m_cache.find(typeIndex);
        if (it == m_cache.end())
        {
            static const CacheBucket emptyBucket{};
            return emptyBucket;
        }
        return it->second;
    }

    bool ResourceManager::unloadTyped(std::type_index typeIndex, const ResourcePath& path)
    {
        const auto it = m_cache.find(typeIndex);
        if (it == m_cache.end())
        {
            return false;
        }

        return it->second.erase(cacheKeyFor(path)) > 0;
    }

    bool ResourceManager::hasTyped(std::type_index typeIndex, const ResourcePath& path) const
    {
        const auto it = m_cache.find(typeIndex);
        if (it == m_cache.end())
        {
            return false;
        }

        const auto resourceIt = it->second.find(cacheKeyFor(path));
        return resourceIt != it->second.end() && !resourceIt->second.expired();
    }

    std::shared_ptr<Resource> ResourceManager::loadTyped(std::type_index typeIndex, ResourceKind kind, const ResourcePath& path)
    {
        auto& bucket = cacheBucket(typeIndex);
        const std::string key = cacheKey(kind, path);
        if (const auto it = bucket.find(key); it != bucket.end())
        {
            if (auto cached = it->second.lock())
            {
                return cached;
            }
        }

        std::shared_ptr<Resource> resource;
        if (path.isBuiltin())
        {
            resource = loadBuiltin(kind, path);
        }
        else if (auto sourcePath = m_vfs.resolve(path))
        {
            resource = loadFromSource(kind, path, *sourcePath);
        }

        if (!resource)
        {
            return nullptr;
        }

        bucket[key] = resource;
        return resource;
    }

    std::shared_ptr<Resource> ResourceManager::loadBuiltin(ResourceKind kind, const ResourcePath& path)
    {
        if (kind == ResourceKind::Model || kind == ResourceKind::Mesh)
        {
            if (auto model = m_modelLoaders.load(path.string()))
            {
                if (kind == ResourceKind::Model)
                {
                    auto resource = std::make_shared<ModelResource>(std::move(*model));
                    resource->virtualPath = path.string();
                    resource->stableId = path.string();
                    resource->loaded = true;
                    return resource;
                }

                if (!model->meshes.empty())
                {
                    return makeMeshResource(path, {}, {}, {}, 0, model->meshes.front());
                }
            }
        }

        if (kind == ResourceKind::Texture || kind == ResourceKind::Image)
        {
            ImageData image = makeCheckerImage();
            if (kind == ResourceKind::Texture)
            {
                auto texture = std::make_shared<TextureResource>();
                texture->image = std::move(image);
                texture->importSettings = TextureImportSettings{};
                texture->virtualPath = path.string();
                texture->stableId = path.string();
                texture->loaded = true;
                return texture;
            }

            auto imageResource = std::make_shared<ImageResource>();
            imageResource->image = std::move(image);
            imageResource->virtualPath = path.string();
            imageResource->stableId = path.string();
            imageResource->loaded = true;
            return imageResource;
        }

        return nullptr;
    }

    std::shared_ptr<Resource> ResourceManager::loadFromBaked(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const ImportMetadata& metadata)
    {
        switch (kind)
        {
        case ResourceKind::Model:
        {
            if (auto model = m_modelLoaders.load(bakedPath))
            {
                auto resource = std::make_shared<ModelResource>(std::move(*model));
                applyResourceMetadata(*resource, path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".modelbin"), metadata.sourceHash);
                resource->importSettings = {};
                resource->dependencies = metadata.dependencies;
                return resource;
            }
            return nullptr;
        }
        case ResourceKind::Mesh:
        {
            if (auto model = m_modelLoaders.load(bakedPath))
            {
                if (!model->meshes.empty())
                {
                    auto resource = makeMeshResource(path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".modelbin"), metadata.sourceHash, model->meshes.front());
                    resource->dependencies = metadata.dependencies;
                    return resource;
                }
            }
            return nullptr;
        }
        case ResourceKind::Texture:
        case ResourceKind::Image:
        {
            std::vector<std::uint8_t> bytes;
            if (!readBinaryBlob(bakedPath, kImageMagic, kImageVersion, bytes))
            {
                return nullptr;
            }
            const auto image = parseImagePayload(bytes);
            if (!image)
            {
                return nullptr;
            }

            if (kind == ResourceKind::Texture)
            {
                auto resource = makeTextureResource(path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".texturebin"), metadata.sourceHash, *image, {});
                resource->dependencies = metadata.dependencies;
                return resource;
            }

            auto resource = makeImageResource(path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".imagebin"), metadata.sourceHash, *image);
            resource->dependencies = metadata.dependencies;
            return resource;
        }
        case ResourceKind::Shader:
        {
            std::string text;
            if (!readTextBlob(bakedPath, kTextMagic, kTextVersion, text))
            {
                return nullptr;
            }

            auto resource = makeShaderResource(path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".shaderbin"), metadata.sourceHash, std::move(text), {});
            resource->dependencies = metadata.dependencies;
            return resource;
        }
        case ResourceKind::Material:
        {
            std::string text;
            if (!readTextBlob(bakedPath, kTextMagic, kTextVersion, text))
            {
                return nullptr;
            }

            auto resource = makeMaterialResource(path, sourcePath, bakedPath, m_vfs.metadataPathFor(path, metadata.sourceHash, ".materialbin"), metadata.sourceHash, std::move(text), {});
            resource->dependencies = metadata.dependencies;
            return resource;
        }
        default:
            break;
        }

        return nullptr;
    }

    std::shared_ptr<Resource> ResourceManager::loadFromSource(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath)
    {
        const std::uint64_t sourceHash = hashFile(sourcePath);
        if (sourceHash == 0)
            return nullptr;

        switch (kind)
        {
        case ResourceKind::Model:
        case ResourceKind::Mesh:
        {
            const auto bakedPath = m_vfs.bakedPathFor(path, sourceHash, ".modelbin");
            const auto metadataPath = m_vfs.metadataPathFor(path, sourceHash, ".modelbin");
            const std::string settingsJson = serializeModelSettings(ModelImportSettings{});

            if (const auto freshMetadata = loadMetadataIfFresh(metadataPath, sourceHash, settingsJson, 1, bakedPath))
                return loadFromBaked(kind, path, sourcePath, bakedPath, *freshMetadata);

            return reimport(kind, path, sourcePath, bakedPath, metadataPath);
        }
        case ResourceKind::Texture:
        case ResourceKind::Image:
        {
            const auto decoded = decodeImageFile(sourcePath);
            if (!decoded)
                return nullptr;


            const auto bakedExtension = kind == ResourceKind::Texture ? ".texturebin" : ".imagebin";
            const auto bakedPath = m_vfs.bakedPathFor(path, sourceHash, bakedExtension);
            const auto metadataPath = m_vfs.metadataPathFor(path, sourceHash, bakedExtension);
            const TextureImportSettings settings{};
            const std::string settingsJson = serializeTextureSettings(settings);

            if (const auto freshMetadata = loadMetadataIfFresh(metadataPath, sourceHash, settingsJson, settings.importerVersion, bakedPath))
                return loadFromBaked(kind, path, sourcePath, bakedPath, *freshMetadata);

            ImageData image = *decoded;
            const auto payload = buildImagePayload(image);
            writeBinaryBlob(bakedPath, kImageMagic, kImageVersion, payload);

            ImportMetadata metadata;
            std::vector<std::string> dependencies;
            stampMetadata(metadata, path, sourcePath, bakedPath, sourceHash, kind == ResourceKind::Texture ? "texture" : "image", settings.importerVersion, settingsJson, std::move(dependencies));
            writeImportMetadata(metadataPath, metadata);

            if (kind == ResourceKind::Texture)
                return makeTextureResource(path, sourcePath, bakedPath, metadataPath, sourceHash, std::move(image), settings);

            return makeImageResource(path, sourcePath, bakedPath, metadataPath, sourceHash, std::move(image));
        }
        case ResourceKind::Shader:
        {
            const auto source = readFileText(sourcePath);
            if (source.empty())
                return nullptr;

            const ShaderImportSettings settings{};
            const auto bakedPath = m_vfs.bakedPathFor(path, sourceHash, ".shaderbin");
            const auto metadataPath = m_vfs.metadataPathFor(path, sourceHash, ".shaderbin");
            const std::string settingsJson = serializeShaderSettings(settings);
            if (const auto freshMetadata = loadMetadataIfFresh(metadataPath, sourceHash, settingsJson, settings.importerVersion, bakedPath))
                return loadFromBaked(kind, path, sourcePath, bakedPath, *freshMetadata);

            writeTextBlob(bakedPath, kTextMagic, kTextVersion, source);
            ImportMetadata metadata;
            stampMetadata(metadata, path, sourcePath, bakedPath, sourceHash, "shader", settings.importerVersion, settingsJson, {});
            writeImportMetadata(metadataPath, metadata);
            return makeShaderResource(path, sourcePath, bakedPath, metadataPath, sourceHash, source, settings);
        }
        case ResourceKind::Material:
        {
            const auto source = readFileText(sourcePath);
            if (source.empty())
                return nullptr;

            const MaterialImportSettings settings{};
            const auto bakedPath = m_vfs.bakedPathFor(path, sourceHash, ".materialbin");
            const auto metadataPath = m_vfs.metadataPathFor(path, sourceHash, ".materialbin");
            const std::string settingsJson = serializeMaterialSettings(settings);
            if (const auto freshMetadata = loadMetadataIfFresh(metadataPath, sourceHash, settingsJson, settings.importerVersion, bakedPath))
                return loadFromBaked(kind, path, sourcePath, bakedPath, *freshMetadata);

            writeTextBlob(bakedPath, kTextMagic, kTextVersion, source);
            ImportMetadata metadata;
            stampMetadata(metadata, path, sourcePath, bakedPath, sourceHash, "material", settings.importerVersion, settingsJson, {});
            writeImportMetadata(metadataPath, metadata);
            return makeMaterialResource(path, sourcePath, bakedPath, metadataPath, sourceHash, source, settings);
        }
        default:
            break;
        }

        return nullptr;
    }

    std::shared_ptr<Resource> ResourceManager::reimport(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath)
    {
        switch (kind)
        {
        case ResourceKind::Model:
        case ResourceKind::Mesh:
        {
            auto model = m_modelLoaders.load(sourcePath);
            if (!model)
                return nullptr;

            ModelImportSettings settings{};
            const std::string settingsJson = serializeModelSettings(settings);
            ImportMetadata metadata;
            stampMetadata(metadata, path, sourcePath, bakedPath, hashFile(sourcePath), "model", settings.importerVersion, settingsJson, {});
            writeBakedModelFile(bakedPath, *model);
            writeImportMetadata(metadataPath, metadata);

            if (kind == ResourceKind::Model)
            {
                auto resource = std::make_shared<ModelResource>(std::move(*model));
                resource->importSettings = settings;
                applyResourceMetadata(*resource, path, sourcePath, bakedPath, metadataPath, metadata.sourceHash);
                return resource;
            }

            if (model->meshes.empty())
            {
                return nullptr;
            }

            auto resource = makeMeshResource(path, sourcePath, bakedPath, metadataPath, metadata.sourceHash, model->meshes.front());
            resource->importSettings = settings;
            return resource;
        }
        default:
            break;
        }

        return nullptr;
    }

    void ResourceManager::importAllStaleAssets()
    {
        const auto root = m_vfs.resolve(ResourcePath::parse("res://"));
        if (!root)
            return;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(*root))
        {
            if (!entry.is_regular_file())
                continue;

            const auto path = entry.path();
            const auto ext = path.extension().string();
            if (ext.empty())
                continue;

            ResourcePath virtualPath;
            virtualPath = ResourcePath::parse("res://" + std::filesystem::relative(path, *root).generic_string());
            if (m_modelLoaders.load(path))
            {
                (void)load<ModelResource>(virtualPath);
                continue;
            }

            if (isImageExtension(path))
            {
                (void)load<TextureResource>(virtualPath);
                continue;
            }

            if (ext == ".sc")
            {
                (void)load<ShaderResource>(virtualPath);
                continue;
            }

            if (ext == ".mat")
            {
                (void)load<MaterialResource>(virtualPath);
                continue;
            }
        }
    }
} // namespace Zenith
