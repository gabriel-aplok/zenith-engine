#pragma once

#include <memory>

#include "math/math.hpp"

#include "engine/debug.hpp"
#include "engine/window.hpp"

namespace Zenith
{

    class RenderContext
    {
    public:
        virtual ~RenderContext() = default;

        virtual GraphicsApi graphicsApi() const = 0;
        virtual IVector2 framebufferSize() const = 0;
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void resize(const IVector2 &framebufferSize) = 0;
    };

    std::unique_ptr<RenderContext> createRenderContext(Window &window, const EngineDebugConfig &debugConfig);

} // namespace Zenith
