#pragma once

#include <memory>
#include <string>

#include "engine/input.hpp"
#include "engine/window_backend.hpp"

namespace Zenith
{
    class Window
    {
    public:
        explicit Window(const WindowConfig &config = {});
        ~Window() = default;

        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;
        Window(Window &&) noexcept = default;
        Window &operator=(Window &&) noexcept = default;

        void pollEvents();
        void swapBuffers();
        bool shouldClose() const;
        void setShouldClose(bool close);

        void setTitle(const std::string &title);
        void setVSync(bool enabled);
        void setCursorMode(CursorMode mode);
        CursorMode getCursorMode() const;

        IVector2 getSize() const;
        IVector2 getFramebufferSize() const;
        float getAspectRatio() const;

        void *getNativeHandle() const;
        const char *getBackendName() const;
        GraphicsApi getGraphicsApi() const;

        InputState &getInput();
        const InputState &getInput() const;

        void setEventCallback(WindowEventCallback callback);

        const WindowConfig &getConfig() const { return m_config; }

    private:
        void dispatchEvent(const WindowEvent &event) const;

        WindowConfig m_config;
        InputState m_input;
        WindowEventCallback m_eventCallback;
        std::unique_ptr<WindowBackend> m_backend;
    };

} // namespace Zenith
