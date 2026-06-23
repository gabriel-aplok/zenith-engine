#pragma once

#include <memory>
#include <string>

#include "resource/resource_id.hpp"

namespace Zenith
{
    template <typename T>
    class ResourceHandle
    {
    public:
        ResourceHandle() = default;
        ResourceHandle(std::shared_ptr<T> resource, std::string path = {})
            : m_resource(std::move(resource)), m_path(std::move(path))
        {
        }

        T *operator->() { return m_resource.get(); }
        const T *operator->() const { return m_resource.get(); }

        T &operator*() { return *m_resource; }
        const T &operator*() const { return *m_resource; }

        explicit operator bool() const { return static_cast<bool>(m_resource); }

        const std::string &path() const { return m_path; }
        std::shared_ptr<T> shared() const { return m_resource; }
        ResourceId id() const { return ResourceId{m_path}; }

        void reset()
        {
            m_resource.reset();
            m_path.clear();
        }

    private:
        std::shared_ptr<T> m_resource;
        std::string m_path;
    };
} // namespace Zenith
