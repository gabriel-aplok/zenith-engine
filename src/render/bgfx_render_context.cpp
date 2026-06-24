#include "render/bgfx_render_context.hpp"

#include <stdexcept>

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include "render/bgfx_debug.hpp"

namespace Zenith
{
    namespace
    {
        bgfx::RendererType::Enum toBgfxRendererType(BgfxRendererBackend backend)
        {
            switch (backend)
            {
            case BgfxRendererBackend::Noop:
                return bgfx::RendererType::Noop;
            case BgfxRendererBackend::Agc:
                return bgfx::RendererType::Agc;
            case BgfxRendererBackend::Direct3D11:
                return bgfx::RendererType::Direct3D11;
            case BgfxRendererBackend::Direct3D12:
                return bgfx::RendererType::Direct3D12;
            case BgfxRendererBackend::Gnm:
                return bgfx::RendererType::Gnm;
            case BgfxRendererBackend::Metal:
                return bgfx::RendererType::Metal;
            case BgfxRendererBackend::Nvn:
                return bgfx::RendererType::Nvn;
            case BgfxRendererBackend::OpenGL:
                return bgfx::RendererType::OpenGL;
            case BgfxRendererBackend::OpenGLES:
                return bgfx::RendererType::OpenGLES;
            case BgfxRendererBackend::Vulkan:
                return bgfx::RendererType::Vulkan;
            case BgfxRendererBackend::Auto:
            default:
                return bgfx::RendererType::Count;
            }
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
        init.type = toBgfxRendererType(window.getConfig().bgfxBackend);
        init.resolution.width = static_cast<uint32_t>(m_framebufferSize.x);
        init.resolution.height = static_cast<uint32_t>(m_framebufferSize.y);
        init.resolution.reset = BGFX_RESET_VSYNC;
        init.platformData.nwh = window.getNativeHandle();

        if (!bgfx::init(init))
        {
            throw std::runtime_error("Failed to initialize bgfx");
        }

        bgfx::setDebug(buildBgfxDebugFlags(m_debugConfig));

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
        logBgfxStartupDiagnostics(window, m_debugConfig);
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

    IVector2 BgfxRenderContext::framebufferSize() const
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

    void BgfxRenderContext::resize(const IVector2 &framebufferSize)
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
