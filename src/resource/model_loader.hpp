#pragma once

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

#include "render/mesh.hpp"

namespace Zenith
{
    class ModelLoaderRegistry
    {
    public:
        using Loader = std::function<std::optional<Render::MeshData>(const std::filesystem::path &)>;

        void registerLoader(const std::string &extension, Loader loader);
        std::optional<Render::MeshData> load(const std::filesystem::path &path) const;
        void clear();

    private:
        static std::string normalizeExtension(std::string extension);

        std::unordered_map<std::string, Loader> m_loaders;
    };

    ModelLoaderRegistry &defaultModelLoaderRegistry();
    void registerStandardModelLoaders(ModelLoaderRegistry &registry);
} // namespace Zenith
