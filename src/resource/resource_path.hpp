#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace Zenith
{
    class ResourcePath
    {
    public:
        ResourcePath() = default;
        explicit ResourcePath(std::string value);

        static ResourcePath parse(std::string_view value);

        const std::string &scheme() const { return m_scheme; }
        const std::string &path() const { return m_path; }

        bool empty() const { return m_scheme.empty() && m_path.empty(); }
        bool hasScheme() const { return !m_scheme.empty(); }
        bool isVirtual() const { return hasScheme(); }
        bool isBuiltin() const { return m_scheme == "builtin"; }
        bool isProject() const { return m_scheme == "res"; }

        std::string string() const;
        std::filesystem::path filename() const;
        std::filesystem::path relativePath() const;

    private:
        std::string m_scheme = "res";
        std::string m_path;
    };
} // namespace Zenith
