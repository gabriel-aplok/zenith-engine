#include "render/bgfx_render_context.hpp"

#include <stdexcept>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

namespace Zenith {

    BgfxRenderContext::BgfxRenderContext(Window& window)
        : m_framebufferSize(window.getFramebufferSize()) {
        if (window.getGraphicsApi() != GraphicsApi::Bgfx) {
            throw std::runtime_error("BgfxRenderContext requires a bgfx window");
        }

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;
        init.resolution.width = static_cast<uint32_t>(m_framebufferSize.x);
        init.resolution.height = static_cast<uint32_t>(m_framebufferSize.y);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData.nwh = window.getNativeHandle();

        if (!bgfx::init(init)) {
            throw std::runtime_error("Failed to initialize bgfx");
        }

        bgfx::reset(
            static_cast<uint32_t>(m_framebufferSize.x),
            static_cast<uint32_t>(m_framebufferSize.y),
            BGFX_RESET_VSYNC);
        bgfx::setViewRect(
            0,
            0,
            0,
            static_cast<uint16_t>(m_framebufferSize.x),
            static_cast<uint16_t>(m_framebufferSize.y));

        m_initialized = true;
    }

    BgfxRenderContext::~BgfxRenderContext() {
        if (m_initialized) {
            bgfx::shutdown();
        }
    }

    GraphicsApi BgfxRenderContext::graphicsApi() const {
        return GraphicsApi::Bgfx;
    }

    glm::ivec2 BgfxRenderContext::framebufferSize() const {
        return m_framebufferSize;
    }

    void BgfxRenderContext::beginFrame() {
        bgfx::setViewRect(
            0,
            0,
            0,
            static_cast<uint16_t>(m_framebufferSize.x),
            static_cast<uint16_t>(m_framebufferSize.y));
    }

    void BgfxRenderContext::endFrame() {
        bgfx::frame();
    }

    void BgfxRenderContext::resize(const glm::ivec2& framebufferSize) {
        m_framebufferSize = framebufferSize;
        if (m_framebufferSize.x <= 0 || m_framebufferSize.y <= 0) {
            return;
        }

        bgfx::reset(
            static_cast<uint32_t>(m_framebufferSize.x),
            static_cast<uint32_t>(m_framebufferSize.y),
            BGFX_RESET_VSYNC);
        bgfx::setViewRect(
            0,
            0,
            0,
            static_cast<uint16_t>(m_framebufferSize.x),
            static_cast<uint16_t>(m_framebufferSize.y));
    }

    std::unique_ptr<RenderContext> createRenderContext(Window& window) {
        return std::make_unique<BgfxRenderContext>(window);
    }

} // namespace Zenith
