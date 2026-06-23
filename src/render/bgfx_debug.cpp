#include "render/bgfx_debug.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/defines.h>

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
    } // namespace

    uint32_t buildBgfxDebugFlags(const EngineDebugConfig &debugConfig)
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

    void logBgfxStartupDiagnostics(const Window &window, const EngineDebugConfig &debugConfig)
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

} // namespace Zenith
