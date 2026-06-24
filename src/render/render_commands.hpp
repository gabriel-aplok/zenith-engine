#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "math/math.hpp"
#include "render/mesh.hpp"

namespace Zenith::Render
{

    class RenderCommandList
    {
    public:
        void clear();
        void reserve(std::size_t drawCount);
        void setView(const RenderViewState &view);
        void setTransform(const Matrix4 &transform);
        void setMaterial(const MaterialState &material);
        void bindMesh(MeshHandle mesh);
        void drawIndexed(uint32_t firstIndex = 0, uint32_t indexCount = 0);
        void drawIndexed(MeshHandle mesh, uint32_t firstIndex = 0, uint32_t indexCount = 0, const Matrix4 &transform = Matrix4{1.0f}, const MaterialState &material = MaterialState{});
        void drawMesh(MeshHandle mesh, const Matrix4 &transform = Matrix4{1.0f}) { drawIndexed(mesh, 0, 0, transform); }
        void finalize();

        const RenderViewState &view() const;
        const std::vector<RenderCommand> &commands() const;
        const std::vector<RenderBatch> &batches() const;
        const RenderStats &stats() const;

    private:
        void markDirty();

        RenderViewState m_view{};
        std::vector<RenderCommand> m_commands{};
        std::vector<RenderBatch> m_batches{};
        RenderStats m_stats{};
        bool m_dirty = true;
    };

} // namespace Zenith::Render
