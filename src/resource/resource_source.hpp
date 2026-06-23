#pragma once

#include <string>
#include <utility>

namespace Zenith
{
    template <typename T>
    class ResourceSource
    {
    public:
        ResourceSource() = default;

        ResourceSource(T data, std::string path = {})
            : m_data(std::move(data)), m_path(std::move(path))
        {
        }

        const T &data() const { return m_data; }
        T &data() { return m_data; }

        const std::string &path() const { return m_path; }

    private:
        T m_data{};
        std::string m_path;
    };
} // namespace Zenith
