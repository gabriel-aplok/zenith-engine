#include "engine/application.hpp"

namespace Zenith
{
    Application::Application(const ApplicationConfig &config)
        : m_config(config)
    {
        m_window = std::make_unique<Window>(m_config.window);
        m_window->setEventCallback([this](const WindowEvent &event)
                                   {
            if (event.type == WindowEventType::CloseRequested) {
                m_running = false;
            }
            onWindowEvent(event); });
        m_lastTime = Clock::now();
    }

    void Application::run()
    {
        onInit();

        while (m_running && !m_window->shouldClose())
        {
            m_window->getInput().beginFrame();
            m_window->pollEvents();

            const auto now = Clock::now();
            const float deltaTime = std::chrono::duration<float>(now - m_lastTime).count();
            m_lastTime = now;

            onUpdate(deltaTime);
            onRender();
            m_window->swapBuffers();
        }

        onShutdown();
    }

    void Application::requestQuit()
    {
        m_running = false;
        m_window->setShouldClose(true);
    }

} // namespace Zenith
