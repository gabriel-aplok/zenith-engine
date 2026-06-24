#pragma once

#include "render/mesh.hpp"
#include "render/render_commands.hpp"

namespace Zenith
{
    struct RenderFrame
    {
        Render::RenderClearState clear{};
        Render::RenderViewState view{};
        Render::RenderCommandList commands{};

        void begin(std::size_t drawCountHint = 0)
        {
            commands.clear();
            if (drawCountHint > 0)
            {
                commands.reserve(drawCountHint);
            }
        }

        void finalize()
        {
            commands.finalize();
        }

        void setView(const Render::RenderViewState &viewState)
        {
            view = viewState;
            commands.setView(viewState);
        }

        void submitMesh(Render::MeshHandle mesh, const Matrix4 &transform = Matrix4{1.0f}, const Render::MaterialState &material = Render::MaterialState{})
        {
            commands.drawIndexed(mesh, 0, 0, transform, material);
        }

        void submitIndexed(Render::MeshHandle mesh, uint32_t firstIndex, uint32_t indexCount, const Matrix4 &transform = Matrix4{1.0f}, const Render::MaterialState &material = Render::MaterialState{})
        {
            commands.drawIndexed(mesh, firstIndex, indexCount, transform, material);
        }

        const Render::RenderStats &stats() const
        {
            return commands.stats();
        }
    };
} // namespace Zenith
