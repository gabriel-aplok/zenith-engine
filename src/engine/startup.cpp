#include "engine/startup.hpp"

#include <algorithm>
#include <string_view>

namespace Zenith
{
    namespace
    {
        BgfxRendererBackend parseBgfxBackend(std::string_view value)
        {
            if (value == "auto")
            {
                return BgfxRendererBackend::Auto;
            }
            if (value == "noop")
            {
                return BgfxRendererBackend::Noop;
            }
            if (value == "agc")
            {
                return BgfxRendererBackend::Agc;
            }
            if (value == "d3d11" || value == "dx11")
            {
                return BgfxRendererBackend::Direct3D11;
            }
            if (value == "d3d12" || value == "dx12")
            {
                return BgfxRendererBackend::Direct3D12;
            }
            if (value == "gnm")
            {
                return BgfxRendererBackend::Gnm;
            }
            if (value == "metal")
            {
                return BgfxRendererBackend::Metal;
            }
            if (value == "nvn")
            {
                return BgfxRendererBackend::Nvn;
            }
            if (value == "opengl" || value == "gl")
            {
                return BgfxRendererBackend::OpenGL;
            }
            if (value == "gles" || value == "opengles")
            {
                return BgfxRendererBackend::OpenGLES;
            }
            if (value == "vulkan" || value == "vk")
            {
                return BgfxRendererBackend::Vulkan;
            }
            return BgfxRendererBackend::Auto;
        }
    } // namespace

    ApplicationConfig parseApplicationConfig(int argc, char **argv)
    {
        ApplicationConfig config{};

        for (int i = 1; i < argc; ++i)
        {
            const std::string_view arg{argv[i] ? argv[i] : ""};

            if (arg == "--no-debug")
            {
                config.debug.enabled = false;
            }
            else if (arg == "--debug")
            {
                config.debug.enabled = true;
            }
            else if (arg == "--no-debug-text")
            {
                config.debug.bgfxText = false;
            }
            else if (arg == "--no-debug-stats")
            {
                config.debug.bgfxStats = false;
            }
            else if (arg == "--debug-text")
            {
                config.debug.bgfxText = true;
            }
            else if (arg == "--debug-stats")
            {
                config.debug.bgfxStats = true;
            }
            else if (arg == "--graphics" && i + 1 < argc)
            {
                config.window.bgfxBackend = parseBgfxBackend(argv[++i] ? argv[i] : "");
            }
            else if (arg.starts_with("--graphics="))
            {
                config.window.bgfxBackend = parseBgfxBackend(arg.substr(std::string_view{"--graphics="}.size()));
            }
        }

        return config;
    }

} // namespace Zenith
