#include "resource/resource_path.hpp"

#include <algorithm>

namespace Zenith
{
    namespace
    {
        std::string normalizeSlashes(std::string value)
        {
            std::replace(value.begin(), value.end(), '\\', '/');
            while (!value.empty() && value.front() == '/')
            {
                value.erase(value.begin());
            }
            return value;
        }
    } // namespace

    ResourcePath::ResourcePath(std::string value)
        : ResourcePath(parse(value))
    {
    }

    ResourcePath ResourcePath::parse(std::string_view value)
    {
        ResourcePath path;
        const std::string text(value);
        const std::size_t schemeEnd = text.find("://");
        if (schemeEnd == std::string::npos)
        {
            path.m_path = normalizeSlashes(text);
            return path;
        }

        path.m_scheme = text.substr(0, schemeEnd);
        path.m_path = normalizeSlashes(text.substr(schemeEnd + 3));
        if (path.m_scheme.empty())
            path.m_scheme = "res";

        return path;
    }

    std::string ResourcePath::string() const
    {
        if (m_scheme.empty())
            return m_path;

        return m_scheme + "://" + m_path;
    }

    std::filesystem::path ResourcePath::filename() const
    {
        return std::filesystem::path{ m_path }.filename();
    }

    std::filesystem::path ResourcePath::relativePath() const
    {
        return std::filesystem::path{ m_path };
    }
} // namespace Zenith
