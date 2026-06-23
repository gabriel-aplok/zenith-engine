#include "engine/application.hpp"
#include "render/bgfx_renderer.hpp"
#include "render/irenderer.hpp"
#include "render/render_context.hpp"

#include <memory>
#include <iostream>

namespace Zenith
{
    class GameApplication final : public Application
    {
    protected:
        void onInit() override
        {
            m_renderContext = createRenderContext(getWindow());
            m_renderer = createRenderer(m_renderContext->graphicsApi());
            m_renderer->initialize(*m_renderContext);
            resizeRenderer(m_renderContext->framebufferSize());
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

int main()
{
    try
    {
        Zenith::GameApplication app;
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
