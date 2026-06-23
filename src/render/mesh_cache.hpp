#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>

#include "render/mesh.hpp"

namespace Zenith::Render
{
    class MeshRef;

    class IMeshUploader
    {
    public:
        virtual ~IMeshUploader() = default;

        virtual MeshHandle uploadMesh(const MeshData &meshData) = 0;
        virtual bool updateMesh(MeshHandle meshHandle, const MeshData &meshData) = 0;
        virtual void destroyMesh(MeshHandle meshHandle) = 0;
    };

    class RenderMeshCache
    {
    public:
        void setUploader(IMeshUploader *uploader);

        MeshHandle acquire(const std::string &key, const MeshData &meshData);
        MeshRef acquireRef(const std::string &key, const MeshData &meshData);
        bool update(const std::string &key, const MeshData &meshData);
        void release(const std::string &key);
        void clear();

        bool has(const std::string &key) const;

    private:
        struct Entry
        {
            MeshHandle handle{};
            uint32_t refCount = 0;
        };

        IMeshUploader *m_uploader = nullptr;
        std::unordered_map<std::string, Entry> m_entries;
    };

    class MeshRef
    {
    public:
        MeshRef() = default;
        MeshRef(RenderMeshCache *cache, std::string key, MeshHandle handle);
        ~MeshRef();

        MeshRef(const MeshRef &) = delete;
        MeshRef &operator=(const MeshRef &) = delete;

        MeshRef(MeshRef &&other) noexcept;
        MeshRef &operator=(MeshRef &&other) noexcept;

        MeshHandle handle() const { return m_handle; }
        explicit operator bool() const { return m_handle.id != 0; }

        void reset();

    private:
        RenderMeshCache *m_cache = nullptr;
        std::string m_key;
        MeshHandle m_handle{};
    };

} // namespace Zenith::Render
