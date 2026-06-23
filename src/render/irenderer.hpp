#pragma once

#include <memory>

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "engine/window_backend.hpp"

namespace Zenith
{
    class RenderContext;

    struct RenderFrame
    {
        glm::vec4 clearColor{0.08f, 0.09f, 0.11f, 1.0f};
    };

    class IRenderer
    {
    public:
        virtual ~IRenderer() = default;

        virtual void initialize(RenderContext &context) = 0;
        virtual void shutdown() = 0;
        virtual void resize(const glm::ivec2 &framebufferSize) = 0;
        virtual void render(const RenderFrame &frame) = 0;
    };

    std::unique_ptr<IRenderer> createRenderer(GraphicsApi api);

} // namespace Zenith
