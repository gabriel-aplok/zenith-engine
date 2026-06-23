#pragma once

#include "render/mesh.hpp"
#include "render/render_commands.hpp"

namespace Zenith
{
    struct RenderFrame
    {
        glm::vec4 clearColor{0.08f, 0.09f, 0.11f, 1.0f};
        Render::RenderCommandList commands{};
    };
} // namespace Zenith
