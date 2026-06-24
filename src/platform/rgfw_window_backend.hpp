#pragma once

#include "engine/input.hpp"
#include "engine/window_backend.hpp"

struct RGFW_window;

namespace Zenith
{
    class InputState;

    class RgfwWindowBackend final : public WindowBackend
    {
    public:
        RgfwWindowBackend(const WindowConfig &config, InputState &input, WindowEventCallback eventCallback);
        ~RgfwWindowBackend() override;

        void pollEvents() override;
        void swapBuffers() override;
        bool shouldClose() const override;
        void setShouldClose(bool close) override;
        void setTitle(const std::string &title) override;
        void setVSync(bool enabled) override;
        glm::ivec2 getSize() const override;
        glm::ivec2 getFramebufferSize() const override;
        float getAspectRatio() const override;
        void *getNativeHandle() const override;
        void setCursorMode(CursorMode mode) override;
        CursorMode getCursorMode() const override;
        const char *getBackendName() const override;
        GraphicsApi getGraphicsApi() const override;

    private:
        static KeyCode mapKeyCode(int key);
        static MouseButton mapMouseButton(int button);

        void dispatchEvent(const WindowEvent &event) const;

        RGFW_window *m_handle = nullptr;
        InputState &m_input;
        WindowEventCallback m_eventCallback;
        CursorMode m_cursorMode = CursorMode::Normal;
        glm::ivec2 m_size{0, 0};
        glm::ivec2 m_framebufferSize{0, 0};
    };

} // namespace Zenith
