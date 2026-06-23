#pragma once

#include <memory>

#include <glm/vec2.hpp>

#include "engine/window.hpp"
#include "render/render_context.hpp"

namespace Zenith {

    class BgfxRenderContext final : public RenderContext {
    public:
        explicit BgfxRenderContext(Window& window);
        ~BgfxRenderContext() override;

        BgfxRenderContext(const BgfxRenderContext&) = delete;
        BgfxRenderContext& operator=(const BgfxRenderContext&) = delete;

        GraphicsApi graphicsApi() const override;
        glm::ivec2 framebufferSize() const override;
        void beginFrame() override;
        void endFrame() override;
        void resize(const glm::ivec2& framebufferSize) override;

    private:
        glm::ivec2 m_framebufferSize{0, 0};
        bool m_initialized = false;
    };

    std::unique_ptr<RenderContext> createRenderContext(Window& window);

} // namespace Zenith
