#pragma once

#include "engine/window_backend.hpp"

struct GLFWwindow;

namespace Zenith
{

    class InputState;

    class GlfwWindowBackend final : public WindowBackend
    {
    public:
        GlfwWindowBackend(const WindowConfig &config, InputState &input, WindowEventCallback eventCallback);
        ~GlfwWindowBackend() override;

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

        static void errorCallback(int error, const char *description);

    private:
        static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
        static void windowSizeCallback(GLFWwindow *window, int width, int height);
        static void windowCloseCallback(GLFWwindow *window);
        static void windowFocusCallback(GLFWwindow *window, int focused);
        static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void cursorPositionCallback(GLFWwindow *window, double x, double y);
        static void scrollCallback(GLFWwindow *window, double xOffset, double yOffset);

        void dispatchEvent(const WindowEvent &event) const;

        GLFWwindow *m_handle = nullptr;
        InputState &m_input;
        WindowEventCallback m_eventCallback;
        CursorMode m_cursorMode = CursorMode::Normal;
    };

} // namespace Zenith
