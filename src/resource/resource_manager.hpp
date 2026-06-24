#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>

#include "resource/import_metadata.hpp"
#include "resource/model_loader.hpp"
#include "resource/resource.hpp"
#include "resource/resource_path.hpp"
#include "resource/virtual_file_system.hpp"

namespace Zenith
{
    class ResourceManager
    {
    public:
        ResourceManager();

        VirtualFileSystem& vfs() { return m_vfs; }
        const VirtualFileSystem& vfs() const { return m_vfs; }

        ModelLoaderRegistry& modelLoaders() { return m_modelLoaders; }
        const ModelLoaderRegistry& modelLoaders() const { return m_modelLoaders; }

        void setProjectRoot(std::filesystem::path root);
        void setUserRoot(std::filesystem::path root);
        void setEditorRoot(std::filesystem::path root);

        template <typename T>
        std::shared_ptr<T> load(const std::string& path)
        {
            return load<T>(ResourcePath::parse(path));
        }

        template <typename T>
        std::shared_ptr<T> load(const ResourcePath& path)
        {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            const auto resource = loadTyped(std::type_index(typeid(T)), ResourceTraits<T>::kind, path);
            return std::dynamic_pointer_cast<T>(resource);
        }

        template <typename T>
        bool unload(const std::string& path)
        {
            return unload<T>(ResourcePath::parse(path));
        }

        template <typename T>
        bool unload(const ResourcePath& path)
        {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            return unloadTyped(std::type_index(typeid(T)), path);
        }

        template <typename T>
        bool has(const std::string& path) const
        {
            return has<T>(ResourcePath::parse(path));
        }

        template <typename T>
        bool has(const ResourcePath& path) const
        {
            static_assert(std::is_base_of_v<Resource, T>, "T must derive from Resource");
            return hasTyped(std::type_index(typeid(T)), path);
        }

        void clear();
        void importAllStaleAssets();

    private:
        using CacheBucket = std::unordered_map<std::string, std::weak_ptr<Resource>>;

        std::shared_ptr<Resource> loadTyped(std::type_index typeIndex, ResourceKind kind, const ResourcePath& path);
        bool unloadTyped(std::type_index typeIndex, const ResourcePath& path);
        bool hasTyped(std::type_index typeIndex, const ResourcePath& path) const;
        std::shared_ptr<Resource> loadBuiltin(ResourceKind kind, const ResourcePath& path);
        std::shared_ptr<Resource> loadFromSource(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath);
        std::shared_ptr<Resource> loadFromBaked(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const ImportMetadata& metadata);
        std::shared_ptr<Resource> reimport(ResourceKind kind, const ResourcePath& path, const std::filesystem::path& sourcePath, const std::filesystem::path& bakedPath, const std::filesystem::path& metadataPath);
        std::string cacheKey(ResourceKind kind, const ResourcePath& path) const;
        CacheBucket& cacheBucket(std::type_index typeIndex);
        const CacheBucket& cacheBucket(std::type_index typeIndex) const;

        VirtualFileSystem m_vfs;
        ModelLoaderRegistry m_modelLoaders;
        std::unordered_map<std::type_index, CacheBucket> m_cache;
    };
} // namespace Zenith
