#include "resource/virtual_file_system.hpp"

#include <algorithm>

namespace Zenith
{
    namespace
    {
        std::filesystem::path appendImportRoot(const std::filesystem::path& root, const ResourcePath& path)
        {
            if (path.path().empty())
                return root;

            return root / path.relativePath();
        }
    } // namespace

    void VirtualFileSystem::setRoot(std::string scheme, std::filesystem::path root)
    {
        scheme = normalizeScheme(std::move(scheme));
        m_roots[scheme] = std::move(root);
    }

    void VirtualFileSystem::clear()
    {
        m_roots.clear();
        m_projectRoot.clear();
        m_cacheRoot.clear();
        m_builtinRoot.clear();
        m_userRoot.clear();
        m_editorRoot.clear();
    }

    void VirtualFileSystem::setProjectRoot(std::filesystem::path root)
    {
        m_projectRoot = std::move(root);
        setRoot("res", m_projectRoot / "resources");
    }

    void VirtualFileSystem::setCacheRoot(std::filesystem::path root)
    {
        m_cacheRoot = std::move(root);
        setRoot("cache", m_cacheRoot);
    }

    void VirtualFileSystem::setBuiltinRoot(std::filesystem::path root)
    {
        m_builtinRoot = std::move(root);
        setRoot("builtin", m_builtinRoot);
    }

    void VirtualFileSystem::setUserRoot(std::filesystem::path root)
    {
        m_userRoot = std::move(root);
        setRoot("user", m_userRoot);
    }

    void VirtualFileSystem::setEditorRoot(std::filesystem::path root)
    {
        m_editorRoot = std::move(root);
        setRoot("editor", m_editorRoot);
    }

    std::optional<std::filesystem::path> VirtualFileSystem::resolve(const ResourcePath& path) const
    {
        const auto root = rootForScheme(path.scheme());
        if (root.empty())
            return std::nullopt;

        if (path.path().empty())
            return root;

        return root / path.relativePath();
    }

    std::filesystem::path VirtualFileSystem::metadataDirectoryFor(const ResourcePath& path) const
    {
        if (auto root = resolve(path))
            return *root;

        return m_cacheRoot;
    }

    std::filesystem::path VirtualFileSystem::importDirectoryFor(const ResourcePath& path) const
    {
        const auto root = path.isProject() ? m_cacheRoot : m_cacheRoot;
        return appendImportRoot(root, path);
    }

    std::filesystem::path VirtualFileSystem::metadataPathFor(const ResourcePath& path, std::uint64_t sourceHash, std::string_view bakedExtension) const
    {
        const auto relative = path.relativePath();
        const auto parent = relative.parent_path();
        const auto stem = relative.stem().string();
        const auto hashText = std::to_string(sourceHash);
        return m_cacheRoot / parent / (stem + "." + hashText + ".import.json");
    }

    std::filesystem::path VirtualFileSystem::bakedPathFor(const ResourcePath& path, std::uint64_t sourceHash, std::string_view bakedExtension) const
    {
        const auto relative = path.relativePath();
        const auto parent = relative.parent_path();
        const auto stem = relative.stem().string();
        std::string ext = std::string(bakedExtension);
        if (!ext.empty() && ext.front() != '.')
        {
            ext.insert(ext.begin(), '.');
        }
        const auto hashText = std::to_string(sourceHash);
        return m_cacheRoot / parent / (stem + "." + hashText + ext);
    }

    std::string VirtualFileSystem::normalizeScheme(std::string scheme)
    {
        std::transform(scheme.begin(), scheme.end(), scheme.begin(), [](unsigned char c)
            { return static_cast<char>(std::tolower(c)); });
        return scheme;
    }

    std::filesystem::path VirtualFileSystem::rootForScheme(std::string_view scheme) const
    {
        const auto it = m_roots.find(normalizeScheme(std::string{ scheme }));
        if (it != m_roots.end())
        {
            return it->second;
        }

        if (scheme.empty())
        {
            return m_projectRoot;
        }

        return {};
    }
} // namespace Zenith
