#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include "render/mesh.hpp"

namespace Zenith::Render {

class IMeshUploader {
public:
    virtual ~IMeshUploader() = default;

    virtual MeshHandle uploadMesh(const MeshData &meshData) = 0;
    virtual void destroyMesh(MeshHandle meshHandle) = 0;
};

class RenderMeshCache {
public:
    void setUploader(IMeshUploader *uploader);

    MeshHandle acquire(const std::string &key, const MeshData &meshData);
    void release(const std::string &key);
    void clear();

    bool has(const std::string &key) const;

private:
    struct Entry {
        MeshHandle handle{};
        uint32_t refCount = 0;
    };

    IMeshUploader *m_uploader = nullptr;
    std::unordered_map<std::string, Entry> m_entries;
};

} // namespace Zenith::Render
