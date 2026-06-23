#pragma once

#include "render/irenderer.hpp"

namespace Zenith {

    class BgfxRenderer final : public IRenderer {
    public:
        BgfxRenderer() = default;

        void initialize(RenderContext& context) override;
        void shutdown() override;
        void resize(const glm::ivec2& framebufferSize) override;
        void render(const RenderFrame& frame) override;

    private:
        glm::ivec2 m_framebufferSize{0, 0};
        bool m_initialized = false;
    };

    std::unique_ptr<IRenderer> createRenderer(GraphicsApi api);

} // namespace Zenith
