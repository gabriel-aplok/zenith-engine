#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "engine/window.hpp"

namespace Zenith
{

    class RenderContext
    {
    public:
        virtual ~RenderContext() = default;

        virtual GraphicsApi graphicsApi() const = 0;
        virtual glm::ivec2 framebufferSize() const = 0;
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void resize(const glm::ivec2 &framebufferSize) = 0;
    };

    std::unique_ptr<RenderContext> createRenderContext(Window &window);

} // namespace Zenith
