#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <functional>

namespace Zenith {

    class Window {
    public:
        using ResizeCallback = std::function<void(int width, int height)>;

        struct Config {
            std::string title = "Zenith Engine";
            int width = 1280;
            int height = 720;
            bool fullscreen = false;
            bool vsync = true;
            bool resizable = true;
        };

        explicit Window(const Config &config = {});
        ~Window();

        // Delete copy
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;

        // Allow move
        Window(Window &&) noexcept;
        Window &operator=(Window &&) noexcept;

        void makeContextCurrent();
        void swapBuffers();
        bool shouldClose() const;
        void setShouldClose(bool close);

        void setTitle(const std::string &title);
        void setVSync(bool enabled);

        glm::ivec2 getSize() const;
        glm::ivec2 getFramebufferSize() const;
        float getAspectRatio() const;

        GLFWwindow *getHandle() const { return m_handle; }

        void setResizeCallback(ResizeCallback callback);

        // Input helpers
        bool isKeyPressed(int key) const;
        bool isMouseButtonPressed(int button) const;
        glm::dvec2 getCursorPos() const;

    private:
        static void glfwErrorCallback(int error, const char *description);
        static void glfwFramebufferResizeCallback(GLFWwindow *window, int width, int height);

        GLFWwindow *m_handle = nullptr;
        ResizeCallback m_resizeCallback;
    };

} // namespace Zenith