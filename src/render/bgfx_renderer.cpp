#include "render/bgfx_renderer.hpp"

#include <algorithm>
#include <memory>
#include <stdexcept>

#include <bgfx/bgfx.h>

#include "render/render_context.hpp"

namespace Zenith {

    namespace {

        uint32_t packRgba8(const glm::vec4& color) {
            const auto r = static_cast<uint32_t>(std::clamp(color.r, 0.0f, 1.0f) * 255.0f);
            const auto g = static_cast<uint32_t>(std::clamp(color.g, 0.0f, 1.0f) * 255.0f);
            const auto b = static_cast<uint32_t>(std::clamp(color.b, 0.0f, 1.0f) * 255.0f);
            const auto a = static_cast<uint32_t>(std::clamp(color.a, 0.0f, 1.0f) * 255.0f);
            return (r << 24) | (g << 16) | (b << 8) | a;
        }

    } // namespace

    void BgfxRenderer::initialize(RenderContext& context) {
        if (context.graphicsApi() != GraphicsApi::Bgfx) {
            throw std::runtime_error("BgfxRenderer requires a bgfx render context");
        }

        m_framebufferSize = context.framebufferSize();
        m_initialized = true;
    }

    void BgfxRenderer::shutdown() {
        m_initialized = false;
        m_framebufferSize = glm::ivec2(0, 0);
    }

    void BgfxRenderer::resize(const glm::ivec2& framebufferSize) {
        m_framebufferSize = framebufferSize;
    }

    void BgfxRenderer::render(const RenderFrame& frame) {
        if (!m_initialized) {
            return;
        }

        bgfx::setViewClear(
            0,
            BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
            packRgba8(frame.clearColor),
            1.0f,
            0);
        bgfx::touch(0);
    }

    std::unique_ptr<IRenderer> createRenderer(GraphicsApi api) {
        switch (api) {
        case GraphicsApi::Bgfx:
            return std::make_unique<BgfxRenderer>();
        case GraphicsApi::None:
        default:
            throw std::runtime_error("No renderer implementation is available for the requested graphics API");
        }
    }

} // namespace Zenith
