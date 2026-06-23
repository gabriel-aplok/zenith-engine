#include "platform/glfw_window_backend.hpp"

#include <stdexcept>
#include <utility>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "engine/input.hpp"
#include "log/log.hpp"

namespace Zenith {

    namespace {

        int g_glfwWindowCount = 0;

        bool isKeyPressedAction(int action) {
            return action == GLFW_PRESS || action == GLFW_REPEAT;
        }

        bool isMouseButtonInRange(int button) {
            return button >= GLFW_MOUSE_BUTTON_1 && button <= GLFW_MOUSE_BUTTON_5;
        }

        KeyCode mapKeyCode(int key) {
            switch (key) {
            case GLFW_KEY_SPACE: return KeyCode::Space;
            case GLFW_KEY_APOSTROPHE: return KeyCode::Apostrophe;
            case GLFW_KEY_COMMA: return KeyCode::Comma;
            case GLFW_KEY_MINUS: return KeyCode::Minus;
            case GLFW_KEY_PERIOD: return KeyCode::Period;
            case GLFW_KEY_SLASH: return KeyCode::Slash;
            case GLFW_KEY_0: return KeyCode::Num0;
            case GLFW_KEY_1: return KeyCode::Num1;
            case GLFW_KEY_2: return KeyCode::Num2;
            case GLFW_KEY_3: return KeyCode::Num3;
            case GLFW_KEY_4: return KeyCode::Num4;
            case GLFW_KEY_5: return KeyCode::Num5;
            case GLFW_KEY_6: return KeyCode::Num6;
            case GLFW_KEY_7: return KeyCode::Num7;
            case GLFW_KEY_8: return KeyCode::Num8;
            case GLFW_KEY_9: return KeyCode::Num9;
            case GLFW_KEY_SEMICOLON: return KeyCode::Semicolon;
            case GLFW_KEY_EQUAL: return KeyCode::Equal;
            case GLFW_KEY_A: return KeyCode::A;
            case GLFW_KEY_B: return KeyCode::B;
            case GLFW_KEY_C: return KeyCode::C;
            case GLFW_KEY_D: return KeyCode::D;
            case GLFW_KEY_E: return KeyCode::E;
            case GLFW_KEY_F: return KeyCode::F;
            case GLFW_KEY_G: return KeyCode::G;
            case GLFW_KEY_H: return KeyCode::H;
            case GLFW_KEY_I: return KeyCode::I;
            case GLFW_KEY_J: return KeyCode::J;
            case GLFW_KEY_K: return KeyCode::K;
            case GLFW_KEY_L: return KeyCode::L;
            case GLFW_KEY_M: return KeyCode::M;
            case GLFW_KEY_N: return KeyCode::N;
            case GLFW_KEY_O: return KeyCode::O;
            case GLFW_KEY_P: return KeyCode::P;
            case GLFW_KEY_Q: return KeyCode::Q;
            case GLFW_KEY_R: return KeyCode::R;
            case GLFW_KEY_S: return KeyCode::S;
            case GLFW_KEY_T: return KeyCode::T;
            case GLFW_KEY_U: return KeyCode::U;
            case GLFW_KEY_V: return KeyCode::V;
            case GLFW_KEY_W: return KeyCode::W;
            case GLFW_KEY_X: return KeyCode::X;
            case GLFW_KEY_Y: return KeyCode::Y;
            case GLFW_KEY_Z: return KeyCode::Z;
            case GLFW_KEY_LEFT_BRACKET: return KeyCode::LeftBracket;
            case GLFW_KEY_BACKSLASH: return KeyCode::Backslash;
            case GLFW_KEY_RIGHT_BRACKET: return KeyCode::RightBracket;
            case GLFW_KEY_GRAVE_ACCENT: return KeyCode::GraveAccent;
            case GLFW_KEY_ESCAPE: return KeyCode::Escape;
            case GLFW_KEY_ENTER: return KeyCode::Enter;
            case GLFW_KEY_TAB: return KeyCode::Tab;
            case GLFW_KEY_BACKSPACE: return KeyCode::Backspace;
            case GLFW_KEY_INSERT: return KeyCode::Insert;
            case GLFW_KEY_DELETE: return KeyCode::Delete;
            case GLFW_KEY_RIGHT: return KeyCode::Right;
            case GLFW_KEY_LEFT: return KeyCode::Left;
            case GLFW_KEY_DOWN: return KeyCode::Down;
            case GLFW_KEY_UP: return KeyCode::Up;
            case GLFW_KEY_PAGE_UP: return KeyCode::PageUp;
            case GLFW_KEY_PAGE_DOWN: return KeyCode::PageDown;
            case GLFW_KEY_HOME: return KeyCode::Home;
            case GLFW_KEY_END: return KeyCode::End;
            case GLFW_KEY_CAPS_LOCK: return KeyCode::CapsLock;
            case GLFW_KEY_SCROLL_LOCK: return KeyCode::ScrollLock;
            case GLFW_KEY_NUM_LOCK: return KeyCode::NumLock;
            case GLFW_KEY_PRINT_SCREEN: return KeyCode::PrintScreen;
            case GLFW_KEY_PAUSE: return KeyCode::Pause;
            case GLFW_KEY_F1: return KeyCode::F1;
            case GLFW_KEY_F2: return KeyCode::F2;
            case GLFW_KEY_F3: return KeyCode::F3;
            case GLFW_KEY_F4: return KeyCode::F4;
            case GLFW_KEY_F5: return KeyCode::F5;
            case GLFW_KEY_F6: return KeyCode::F6;
            case GLFW_KEY_F7: return KeyCode::F7;
            case GLFW_KEY_F8: return KeyCode::F8;
            case GLFW_KEY_F9: return KeyCode::F9;
            case GLFW_KEY_F10: return KeyCode::F10;
            case GLFW_KEY_F11: return KeyCode::F11;
            case GLFW_KEY_F12: return KeyCode::F12;
            case GLFW_KEY_LEFT_SHIFT: return KeyCode::LeftShift;
            case GLFW_KEY_LEFT_CONTROL: return KeyCode::LeftControl;
            case GLFW_KEY_LEFT_ALT: return KeyCode::LeftAlt;
            case GLFW_KEY_RIGHT_SHIFT: return KeyCode::RightShift;
            case GLFW_KEY_RIGHT_CONTROL: return KeyCode::RightControl;
            case GLFW_KEY_RIGHT_ALT: return KeyCode::RightAlt;
            default: return KeyCode::Unknown;
            }
        }

        MouseButton mapMouseButton(int button) {
            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT: return MouseButton::Left;
            case GLFW_MOUSE_BUTTON_RIGHT: return MouseButton::Right;
            case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButton::Middle;
            case GLFW_MOUSE_BUTTON_4: return MouseButton::Button4;
            case GLFW_MOUSE_BUTTON_5: return MouseButton::Button5;
            default: return MouseButton::Left;
            }
        }

        int toGlfwCursorMode(CursorMode mode) {
            switch (mode) {
            case CursorMode::Hidden: return GLFW_CURSOR_HIDDEN;
            case CursorMode::Disabled: return GLFW_CURSOR_DISABLED;
            case CursorMode::Normal:
            default:
                return GLFW_CURSOR_NORMAL;
            }
        }

        void initializeGlfw() {
            if (g_glfwWindowCount == 0) {
                glfwSetErrorCallback([](int error, const char* description) {
                    GlfwWindowBackend::errorCallback(error, description);
                });
                if (!glfwInit()) {
                    throw std::runtime_error("Failed to initialize GLFW");
                }
            }
            ++g_glfwWindowCount;
        }

        void shutdownGlfw() {
            --g_glfwWindowCount;
            if (g_glfwWindowCount == 0) {
                glfwTerminate();
            }
        }

    } // namespace

    GlfwWindowBackend::GlfwWindowBackend(const WindowConfig& config, InputState& input, WindowEventCallback eventCallback)
        : m_input(input), m_eventCallback(std::move(eventCallback)) {
        initializeGlfw();

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

        if (config.graphicsApi != GraphicsApi::Bgfx && config.graphicsApi != GraphicsApi::None) {
            shutdownGlfw();
            throw std::runtime_error("Unsupported graphics API requested for GLFW backend");
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        GLFWmonitor* monitor = config.fullscreen ? glfwGetPrimaryMonitor() : nullptr;
        m_handle = glfwCreateWindow(config.width, config.height, config.title.c_str(), monitor, nullptr);
        if (!m_handle) {
            shutdownGlfw();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(m_handle, this);
        glfwSetWindowSizeCallback(m_handle, windowSizeCallback);
        glfwSetFramebufferSizeCallback(m_handle, framebufferResizeCallback);
        glfwSetWindowCloseCallback(m_handle, windowCloseCallback);
        glfwSetWindowFocusCallback(m_handle, windowFocusCallback);
        glfwSetKeyCallback(m_handle, keyCallback);
        glfwSetMouseButtonCallback(m_handle, mouseButtonCallback);
        glfwSetCursorPosCallback(m_handle, cursorPositionCallback);
        glfwSetScrollCallback(m_handle, scrollCallback);

        setVSync(config.vsync);
        setCursorMode(CursorMode::Normal);
    }

    GlfwWindowBackend::~GlfwWindowBackend() {
        if (m_handle) {
            glfwDestroyWindow(m_handle);
            m_handle = nullptr;
        }
        shutdownGlfw();
    }

    void GlfwWindowBackend::pollEvents() {
        glfwPollEvents();
    }

    void GlfwWindowBackend::swapBuffers() {
        // bgfx owns presentation; GLFW only handles window events here.
    }

    bool GlfwWindowBackend::shouldClose() const {
        return glfwWindowShouldClose(m_handle) == GLFW_TRUE;
    }

    void GlfwWindowBackend::setShouldClose(bool close) {
        glfwSetWindowShouldClose(m_handle, close ? GLFW_TRUE : GLFW_FALSE);
    }

    void GlfwWindowBackend::setTitle(const std::string& title) {
        glfwSetWindowTitle(m_handle, title.c_str());
    }

    void GlfwWindowBackend::setVSync(bool enabled) {
        (void)enabled;
    }

    glm::ivec2 GlfwWindowBackend::getSize() const {
        glm::ivec2 size{0, 0};
        glfwGetWindowSize(m_handle, &size.x, &size.y);
        return size;
    }

    glm::ivec2 GlfwWindowBackend::getFramebufferSize() const {
        glm::ivec2 size{0, 0};
        glfwGetFramebufferSize(m_handle, &size.x, &size.y);
        return size;
    }

    float GlfwWindowBackend::getAspectRatio() const {
        const auto size = getFramebufferSize();
        return size.y == 0 ? 0.0f : static_cast<float>(size.x) / static_cast<float>(size.y);
    }

    void* GlfwWindowBackend::getNativeHandle() const {
#if defined(_WIN32)
        return glfwGetWin32Window(m_handle);
#else
        return m_handle;
#endif
    }

    void GlfwWindowBackend::setCursorMode(CursorMode mode) {
        m_cursorMode = mode;
        glfwSetInputMode(m_handle, GLFW_CURSOR, toGlfwCursorMode(mode));
    }

    CursorMode GlfwWindowBackend::getCursorMode() const {
        return m_cursorMode;
    }

    const char* GlfwWindowBackend::getBackendName() const {
        return "GLFW";
    }

    GraphicsApi GlfwWindowBackend::getGraphicsApi() const {
        return GraphicsApi::Bgfx;
    }

    void GlfwWindowBackend::errorCallback(int error, const char* description) {
        Log::Error("GLFW Error {}: {}", error, description ? description : "unknown");
    }

    void GlfwWindowBackend::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->dispatchEvent(WindowEvent{
            .type = WindowEventType::FramebufferResized,
            .size = {width, height}
        });
    }

    void GlfwWindowBackend::windowSizeCallback(GLFWwindow* window, int width, int height) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->dispatchEvent(WindowEvent{
            .type = WindowEventType::Resized,
            .size = {width, height}
        });
    }

    void GlfwWindowBackend::windowCloseCallback(GLFWwindow* window) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->dispatchEvent(WindowEvent{.type = WindowEventType::CloseRequested});
    }

    void GlfwWindowBackend::windowFocusCallback(GLFWwindow* window, int focused) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->m_input.setFocused(focused == GLFW_TRUE);
        self->dispatchEvent(WindowEvent{
            .type = WindowEventType::FocusChanged,
            .focused = focused == GLFW_TRUE
        });
    }

    void GlfwWindowBackend::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        (void)scancode;
        (void)mods;

        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self || action == GLFW_REPEAT) {
            return;
        }

        self->m_input.onKeyEvent(mapKeyCode(key), isKeyPressedAction(action));
    }

    void GlfwWindowBackend::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        (void)mods;

        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self || !isMouseButtonInRange(button)) {
            return;
        }

        self->m_input.onMouseButtonEvent(mapMouseButton(button), isKeyPressedAction(action));
    }

    void GlfwWindowBackend::cursorPositionCallback(GLFWwindow* window, double x, double y) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->m_input.onMouseMove(x, y);
    }

    void GlfwWindowBackend::scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
        auto* self = static_cast<GlfwWindowBackend*>(glfwGetWindowUserPointer(window));
        if (!self) {
            return;
        }

        self->m_input.onMouseScroll(xOffset, yOffset);
    }

    void GlfwWindowBackend::dispatchEvent(const WindowEvent& event) const {
        if (m_eventCallback) {
            m_eventCallback(event);
        }
    }

    std::unique_ptr<WindowBackend> createDefaultWindowBackend(
        const WindowConfig& config,
        InputState& input,
        WindowEventCallback eventCallback) {
        return std::make_unique<GlfwWindowBackend>(config, input, std::move(eventCallback));
    }

} // namespace Zenith
