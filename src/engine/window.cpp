#include "engine/window.hpp"
#include <stdexcept>
#include <iostream>

namespace Zenith {

    Window::Window(const Config& config) {
        // Set error callback
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        // OpenGL 4.6 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GL_TRUE : GL_FALSE);

        // Create window
        GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
        m_handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
        if (!m_handle) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        makeContextCurrent();

        // Load OpenGL
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        std::cout << "OpenGL " << GLVersion.major << "." << GLVersion.minor << " loaded\n";

        // Set callbacks
        glfwSetWindowUserPointer(m_handle, this);
        glfwSetFramebufferSizeCallback(m_handle, glfwFramebufferResizeCallback);

        setVSync(config.vsync);
    }

    Window::~Window() {
        if (m_handle) {
            glfwDestroyWindow(m_handle);
        }
        glfwTerminate();
    }

    Window::Window(Window&& other) noexcept
        : m_handle(other.m_handle), m_resizeCallback(std::move(other.m_resizeCallback)) {
        other.m_handle = nullptr;
    }

    Window& Window::operator=(Window&& other) noexcept {
        if (this != &other) {
            if (m_handle) glfwDestroyWindow(m_handle);
            m_handle = other.m_handle;
            m_resizeCallback = std::move(other.m_resizeCallback);
            other.m_handle = nullptr;
        }
        return *this;
    }

    void Window::makeContextCurrent() {
        glfwMakeContextCurrent(m_handle);
    }

    void Window::swapBuffers() {
        glfwSwapBuffers(m_handle);
    }

    bool Window::shouldClose() const {
        return glfwWindowShouldClose(m_handle);
    }

    void Window::setShouldClose(bool close) {
        glfwSetWindowShouldClose(m_handle, close ? GLFW_TRUE : GLFW_FALSE);
    }

    void Window::setTitle(const std::string& title) {
        glfwSetWindowTitle(m_handle, title.c_str());
    }

    void Window::setVSync(bool enabled) {
        glfwSwapInterval(enabled ? 1 : 0);
    }

    glm::ivec2 Window::getSize() const {
        glm::ivec2 size;
        glfwGetWindowSize(m_handle, &size.x, &size.y);
        return size;
    }

    glm::ivec2 Window::getFramebufferSize() const {
        glm::ivec2 size;
        glfwGetFramebufferSize(m_handle, &size.x, &size.y);
        return size;
    }

    float Window::getAspectRatio() const {
        auto size = getFramebufferSize();
        return static_cast<float>(size.x) / static_cast<float>(size.y);
    }

    void Window::setResizeCallback(ResizeCallback callback) {
        m_resizeCallback = std::move(callback);
    }

    bool Window::isKeyPressed(int key) const {
        return glfwGetKey(m_handle, key) == GLFW_PRESS;
    }

    bool Window::isMouseButtonPressed(int button) const {
        return glfwGetMouseButton(m_handle, button) == GLFW_PRESS;
    }

    glm::dvec2 Window::getCursorPos() const {
        glm::dvec2 pos;
        glfwGetCursorPos(m_handle, &pos.x, &pos.y);
        return pos;
    }

    void Window::glfwErrorCallback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    void Window::glfwFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto* self = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (self && self->m_resizeCallback) {
            self->m_resizeCallback(width, height);
        }
        glViewport(0, 0, width, height);
    }

} // namespace Zenith