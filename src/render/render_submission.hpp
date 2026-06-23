#pragma once

#include "render/mesh.hpp"
#include "render/render_commands.hpp"

namespace Zenith
{
    struct RenderFrame
    {
        glm::vec4 clearColor{0.08f, 0.09f, 0.11f, 1.0f};
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

        void setView(const Render::RenderViewState &viewState)
        {
            view = viewState;
            commands.setView(viewState);
        }

        void submitMesh(Render::MeshHandle mesh, const glm::mat4 &transform = glm::mat4{1.0f}, const Render::MaterialState &material = Render::MaterialState{})
        {
            commands.drawIndexed(mesh, 0, 0, transform, material);
        }

        void submitIndexed(Render::MeshHandle mesh, uint32_t firstIndex, uint32_t indexCount, const glm::mat4 &transform = glm::mat4{1.0f}, const Render::MaterialState &material = Render::MaterialState{})
        {
            commands.drawIndexed(mesh, firstIndex, indexCount, transform, material);
        }
    };
} // namespace Zenith
