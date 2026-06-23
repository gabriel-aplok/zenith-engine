#include "engine/application.hpp"
#include "log/log.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/render_context.hpp"

#include <string_view>
#include <memory>
#include <iostream>

namespace
{
    Zenith::ApplicationConfig parseCommandLine(int argc, char **argv)
    {
        Zenith::ApplicationConfig config{};

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
        }

        return config;
    }
} // namespace

namespace Zenith
{
    class GameApplication final : public Application
    {
    public:
        explicit GameApplication(const ApplicationConfig &config)
            : Application(config)
        {
        }

    protected:
        void onInit() override
        {
            const auto &debugConfig = getConfig().debug;

            m_renderContext = createRenderContext(getWindow(), debugConfig);
            m_renderer = createRenderer(m_renderContext->graphicsApi());
            m_renderer->initialize(*m_renderContext);
            resizeRenderer(m_renderContext->framebufferSize());

            if (debugConfig.enabled && debugConfig.logStartupInfo)
            {
                Log::Info("Window title: {}", getConfig().window.title);
                Log::Info("Fullscreen: {}, VSync: {}, Resizable: {}",
                          getConfig().window.fullscreen,
                          getConfig().window.vsync,
                          getConfig().window.resizable);
            }
        }

        void onRender() override
        {
            if (!m_renderContext || !m_renderer)
            {
                return;
            }

            m_renderContext->beginFrame();
            m_renderer->render(m_frame);
            m_renderContext->endFrame();
        }

        void onShutdown() override
        {
            if (m_renderer)
            {
                m_renderer->shutdown();
                m_renderer.reset();
            }
            m_renderContext.reset();
        }

        void onWindowEvent(const WindowEvent &event) override
        {
            if (event.type == WindowEventType::FramebufferResized)
            {
                resizeRenderer(event.size);
            }
        }

    private:
        void resizeRenderer(const glm::ivec2 &framebufferSize)
        {
            if (!m_renderContext || !m_renderer || framebufferSize.x <= 0 || framebufferSize.y <= 0)
            {
                return;
            }

            m_renderContext->resize(framebufferSize);
            m_renderer->resize(framebufferSize);
        }

        std::unique_ptr<RenderContext> m_renderContext;
        std::unique_ptr<IRenderer> m_renderer;
        RenderFrame m_frame;
    };

} // namespace Zenith

int main(int argc, char **argv)
{
    try
    {
        Zenith::Log::Init();
        Zenith::GameApplication app{parseCommandLine(argc, argv)};
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
