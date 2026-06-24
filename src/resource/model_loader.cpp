#include "resource/model_loader.hpp"

#include <algorithm>

#include "render/mesh_builder.hpp"
#include "resource/obj_mesh_loader.hpp"

namespace Zenith
{
    namespace
    {
        std::optional<Render::MeshData> loadBuiltinMesh(const std::filesystem::path &path)
        {
            const std::string name = path.string();
            if (name == "builtin://cube")
                return Zenith::MeshBuilder::makeCube();

            if (name == "builtin://pyramid")
                return Zenith::MeshBuilder::makePyramid();

            if (name == "builtin://plane")
                return Zenith::MeshBuilder::makePlane();

            return std::nullopt;
        }

        std::optional<Render::MeshData> loadObjModel(const std::filesystem::path &path)
        {
            return loadObjMesh(path);
        }

        std::optional<Render::MeshData> loadZenithBakedModel(const std::filesystem::path &path)
        {
            (void)path;
            return std::nullopt;
        }
    } // namespace

    void ModelLoaderRegistry::registerLoader(const std::string &extension, Loader loader)
    {
        m_loaders[normalizeExtension(extension)] = std::move(loader);
    }

    std::optional<Render::MeshData> ModelLoaderRegistry::load(const std::filesystem::path &path) const
    {
        if (auto builtin = loadBuiltinMesh(path))
        {
            return builtin;
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

    ModelLoaderRegistry &defaultModelLoaderRegistry()
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

    void registerStandardModelLoaders(ModelLoaderRegistry &registry)
    {
        registry.registerLoader(".obj", loadObjModel);
        registry.registerLoader(".zenith", loadZenithBakedModel);
        registry.registerLoader(".zmesh", loadZenithBakedModel);
    }
} // namespace Zenith
