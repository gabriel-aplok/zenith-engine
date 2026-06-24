#include "engine/window.hpp"

#include <stdexcept>
#include <utility>

namespace Zenith
{
    Window::Window(const WindowConfig &config)
        : m_config(config)
    {
        m_backend = createDefaultWindowBackend(
            m_config,
            m_input,
            [this](const WindowEvent &event)
            { dispatchEvent(event); });

        if (!m_backend)
        {
            throw std::runtime_error("Failed to create a window backend");
        }
    }

    void Window::pollEvents()
    {
        m_backend->pollEvents();
    }

    void Window::swapBuffers()
    {
        m_backend->swapBuffers();
    }

    bool Window::shouldClose() const
    {
        return m_backend->shouldClose();
    }

    void Window::setShouldClose(bool close)
    {
        m_backend->setShouldClose(close);
    }

    void Window::setTitle(const std::string &title)
    {
        m_backend->setTitle(title);
    }

    void Window::setVSync(bool enabled)
    {
        m_backend->setVSync(enabled);
    }

    void Window::setCursorMode(CursorMode mode)
    {
        m_backend->setCursorMode(mode);
    }

    CursorMode Window::getCursorMode() const
    {
        return m_backend->getCursorMode();
    }

    IVector2 Window::getSize() const
    {
        return m_backend->getSize();
    }

    IVector2 Window::getFramebufferSize() const
    {
        return m_backend->getFramebufferSize();
    }

    float Window::getAspectRatio() const
    {
        return m_backend->getAspectRatio();
    }

    void *Window::getNativeHandle() const
    {
        return m_backend->getNativeHandle();
    }

    const char *Window::getBackendName() const
    {
        return m_backend->getBackendName();
    }

    GraphicsApi Window::getGraphicsApi() const
    {
        return m_backend->getGraphicsApi();
    }

    InputState &Window::getInput()
    {
        return m_input;
    }

    const InputState &Window::getInput() const
    {
        return m_input;
    }

    void Window::setEventCallback(WindowEventCallback callback)
    {
        m_eventCallback = std::move(callback);
    }

    void Window::dispatchEvent(const WindowEvent &event) const
    {
        if (m_eventCallback)
        {
            m_eventCallback(event);
        }
    }

} // namespace Zenith
