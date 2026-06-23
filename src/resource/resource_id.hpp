#pragma once

#include <string>
#include <utility>

namespace Zenith
{
    class ResourceId
    {
    public:
        ResourceId() = default;
        explicit ResourceId(std::string path)
            : m_path(std::move(path))
        {
        }

        const std::string &path() const { return m_path; }
        explicit operator bool() const { return !m_path.empty(); }

    private:
        std::string m_path;
    };
} // namespace Zenith
