#include "render/bgfx_render_context.hpp"

#include <cstdint>
#include <stdexcept>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>

#include "engine/debug.hpp"
#include "log/log.hpp"

namespace Zenith
{

    namespace
    {

        const char *graphicsApiName(GraphicsApi api)
        {
            switch (api)
            {
            case GraphicsApi::Bgfx:
                return "bgfx";
            case GraphicsApi::None:
            default:
                return "none";
            }
        }

        uint32_t buildDebugFlags(const EngineDebugConfig &debugConfig)
        {
            if (!debugConfig.enabled)
            {
                return BGFX_DEBUG_NONE;
            }

            uint32_t flags = BGFX_DEBUG_NONE;
            if (debugConfig.bgfxStats)
            {
                flags |= BGFX_DEBUG_STATS;
            }
            if (debugConfig.bgfxText)
            {
                flags |= BGFX_DEBUG_TEXT;
            }
            return flags;
        }

        void logStartupDiagnostics(const Window &window, const EngineDebugConfig &debugConfig)
        {
            if (!debugConfig.enabled || !debugConfig.logStartupInfo)
            {
                return;
            }

            const auto rendererType = bgfx::getRendererType();
            const auto *caps = bgfx::getCaps();

            Log::Info("Zenith startup diagnostics");
            Log::Info("Window backend: {}", window.getBackendName());
            Log::Info("Graphics API: {}", graphicsApiName(window.getGraphicsApi()));
            Log::Info("Window size: {}x{}", window.getSize().x, window.getSize().y);
            Log::Info("Framebuffer size: {}x{}", window.getFramebufferSize().x, window.getFramebufferSize().y);
            Log::Info("bgfx API version: {}", BGFX_API_VERSION);
            Log::Info("bgfx renderer: {}", bgfx::getRendererName(rendererType));

            if (!caps)
            {
                Log::Warn("bgfx caps are unavailable after initialization");
                return;
            }

            Log::Info("bgfx adapter: vendor=0x{:04x}, device=0x{:04x}", caps->vendorId, caps->deviceId);
            Log::Info(
                "bgfx flags: multithreaded={}, hidpi={}, debug={}",
                (caps->supported & BGFX_CAPS_RENDERER_MULTITHREADED) != 0,
                (caps->supported & BGFX_CAPS_HIDPI) != 0,
                (caps->supported & BGFX_CAPS_GRAPHICS_DEBUGGER) != 0);
            Log::Info(
                "bgfx limits: views={}, drawCalls={}, textures={}, textureSize={}",
                caps->limits.maxViews,
                caps->limits.maxDrawCalls,
                caps->limits.maxTextures,
                caps->limits.maxTextureSize);
        }

    } // namespace

    BgfxRenderContext::BgfxRenderContext(Window &window, const EngineDebugConfig &debugConfig)
        : m_framebufferSize(window.getFramebufferSize()), m_debugConfig(debugConfig)
    {
        if (window.getGraphicsApi() != GraphicsApi::Bgfx)
        {
            throw std::runtime_error("BgfxRenderContext requires a bgfx window");
        }

        bgfx::Init init;
        init.type = bgfx::RendererType::Count;
        init.resolution.width = static_cast<uint32_t>(m_framebufferSize.x);
        init.resolution.height = static_cast<uint32_t>(m_framebufferSize.y);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData.nwh = window.getNativeHandle();

        if (!bgfx::init(init))
        {
            throw std::runtime_error("Failed to initialize bgfx");
        }

        bgfx::setDebug(buildDebugFlags(m_debugConfig));

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
        logStartupDiagnostics(window, m_debugConfig);
    }

    BgfxRenderContext::~BgfxRenderContext()
    {
        if (m_initialized)
        {
            bgfx::shutdown();
        }
    }

    GraphicsApi BgfxRenderContext::graphicsApi() const
    {
        return GraphicsApi::Bgfx;
    }

    glm::ivec2 BgfxRenderContext::framebufferSize() const
    {
        return m_framebufferSize;
    }

    void BgfxRenderContext::beginFrame()
    {
        bgfx::setViewRect(
            0,
            0,
            0,
            static_cast<uint16_t>(m_framebufferSize.x),
            static_cast<uint16_t>(m_framebufferSize.y));
    }

    void BgfxRenderContext::endFrame()
    {
        bgfx::frame();
    }

    void BgfxRenderContext::resize(const glm::ivec2 &framebufferSize)
    {
        m_framebufferSize = framebufferSize;
        if (m_framebufferSize.x <= 0 || m_framebufferSize.y <= 0)
        {
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

    std::unique_ptr<RenderContext> createRenderContext(Window &window, const EngineDebugConfig &debugConfig)
    {
        return std::make_unique<BgfxRenderContext>(window, debugConfig);
    }

} // namespace Zenith
