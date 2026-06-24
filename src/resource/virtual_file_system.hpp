#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>

#include "resource/resource_path.hpp"

namespace Zenith
{
    class VirtualFileSystem
    {
    public:
        void setRoot(std::string scheme, std::filesystem::path root);
        void clear();

        void setProjectRoot(std::filesystem::path root);
        void setCacheRoot(std::filesystem::path root);
        void setBuiltinRoot(std::filesystem::path root);
        void setUserRoot(std::filesystem::path root);
        void setEditorRoot(std::filesystem::path root);

        const std::filesystem::path& projectRoot() const { return m_projectRoot; }
        const std::filesystem::path& cacheRoot() const { return m_cacheRoot; }
        const std::filesystem::path& builtinRoot() const { return m_builtinRoot; }
        const std::filesystem::path& userRoot() const { return m_userRoot; }
        const std::filesystem::path& editorRoot() const { return m_editorRoot; }

        std::optional<std::filesystem::path> resolve(const ResourcePath& path) const;
        std::filesystem::path metadataDirectoryFor(const ResourcePath& path) const;
        std::filesystem::path importDirectoryFor(const ResourcePath& path) const;
        std::filesystem::path metadataPathFor(const ResourcePath& path, std::uint64_t sourceHash, std::string_view bakedExtension) const;
        std::filesystem::path bakedPathFor(const ResourcePath& path, std::uint64_t sourceHash, std::string_view bakedExtension) const;

        static std::string normalizeScheme(std::string scheme);

    private:
        std::filesystem::path rootForScheme(std::string_view scheme) const;

        std::filesystem::path m_projectRoot;
        std::filesystem::path m_cacheRoot;
        std::filesystem::path m_builtinRoot;
        std::filesystem::path m_userRoot;
        std::filesystem::path m_editorRoot;
        std::unordered_map<std::string, std::filesystem::path> m_roots;
    };
} // namespace Zenith
