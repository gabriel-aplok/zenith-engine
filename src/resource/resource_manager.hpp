#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>

#include "resource/resource_handle.hpp"

namespace Zenith
{
    class ResourceManager
    {
    public:
        template <typename T>
        using Loader = std::function<std::shared_ptr<T>(const std::string &path)>;

        template <typename T>
        void registerLoader(Loader<T> loader)
        {
            m_loaders[typeid(T)] = [loader = std::move(loader)](const std::string &path) -> std::shared_ptr<void> {
                return loader ? loader(path) : nullptr;
            };
        }

        template <typename T>
        ResourceHandle<T> load(const std::string &path)
        {
            auto &cache = m_entries[typeid(T)];
            const auto it = cache.resources.find(path);
            if (it != cache.resources.end())
            {
                if (auto existing = it->second.lock())
                {
                    return ResourceHandle<T>{std::static_pointer_cast<T>(existing), path};
                }
            }

            const auto loaderIt = m_loaders.find(typeid(T));
            if (loaderIt == m_loaders.end())
            {
                return {};
            }

            std::shared_ptr<void> resource = loaderIt->second(path);
            if (!resource)
            {
                return {};
            }

            cache.resources[path] = resource;
            return ResourceHandle<T>{std::static_pointer_cast<T>(resource), path};
        }

        template <typename T>
        ResourceHandle<T> store(const std::string &path, std::shared_ptr<T> resource)
        {
            if (!resource)
            {
                return {};
            }

            auto &cache = m_entries[typeid(T)];
            cache.resources[path] = resource;
            return ResourceHandle<T>{std::move(resource), path};
        }

        template <typename T>
        bool unload(const std::string &path)
        {
            const auto it = m_entries.find(typeid(T));
            if (it == m_entries.end())
            {
                return false;
            }

            return it->second.resources.erase(path) > 0;
        }

        void clear()
        {
            m_entries.clear();
        }

        template <typename T>
        bool has(const std::string &path) const
        {
            const auto it = m_entries.find(typeid(T));
            if (it == m_entries.end())
            {
                return false;
            }

            const auto resourceIt = it->second.resources.find(path);
            return resourceIt != it->second.resources.end() && !resourceIt->second.expired();
        }

    private:
        struct TypeEntry
        {
            std::unordered_map<std::string, std::weak_ptr<void>> resources;
        };

        std::unordered_map<std::type_index, TypeEntry> m_entries;
        std::unordered_map<std::type_index, std::function<std::shared_ptr<void>(const std::string &path)>> m_loaders;
    };
} // namespace Zenith
