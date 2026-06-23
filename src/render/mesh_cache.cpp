#include "render/mesh_cache.hpp"

namespace Zenith::Render
{

    void RenderMeshCache::setUploader(IMeshUploader *uploader)
    {
        m_uploader = uploader;
    }

    MeshHandle RenderMeshCache::acquire(const std::string &key, const MeshData &meshData)
    {
        if (!m_uploader)
        {
            return {};
        }

        auto [it, inserted] = m_entries.try_emplace(key);
        Entry &entry = it->second;
        if (inserted || entry.handle.id == 0)
        {
            entry.handle = m_uploader->uploadMesh(meshData);
            entry.refCount = entry.handle.id != 0 ? 1u : 0u;
            if (entry.handle.id == 0)
            {
                m_entries.erase(it);
                return {};
            }
            return entry.handle;
        }

        ++entry.refCount;
        return entry.handle;
    }

    bool RenderMeshCache::update(const std::string &key, const MeshData &meshData)
    {
        if (!m_uploader)
        {
            return false;
        }

        const auto it = m_entries.find(key);
        if (it == m_entries.end())
        {
            return false;
        }

        return m_uploader->updateMesh(it->second.handle, meshData);
    }

    void RenderMeshCache::release(const std::string &key)
    {
        const auto it = m_entries.find(key);
        if (it == m_entries.end())
        {
            return;
        }

        Entry &entry = it->second;
        if (entry.refCount > 0)
        {
            --entry.refCount;
        }

        if (entry.refCount == 0 && entry.handle.id != 0)
        {
            m_uploader->destroyMesh(entry.handle);
            m_entries.erase(it);
        }
    }

    void RenderMeshCache::clear()
    {
        if (!m_uploader)
        {
            m_entries.clear();
            return;
        }

        for (auto &entryPair : m_entries)
        {
            Entry &entry = entryPair.second;
            if (entry.handle.id != 0)
            {
                m_uploader->destroyMesh(entry.handle);
            }
        }
        m_entries.clear();
    }

    bool RenderMeshCache::has(const std::string &key) const
    {
        return m_entries.find(key) != m_entries.end();
    }

} // namespace Zenith::Render
