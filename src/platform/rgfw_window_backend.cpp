#define RGFW_IMPLEMENTATION
#include "RGFW.h"

#include "platform/rgfw_window_backend.hpp"

#include <stdexcept>
#include <utility>

#include "engine/input.hpp"
#include "log/log.hpp"

namespace Zenith
{
    namespace
    {
        bool isKeyPressedEvent(const RGFW_eventType type)
        {
            return type == RGFW_keyPressed;
        }

        bool isMousePressedEvent(const RGFW_eventType type)
        {
            return type == RGFW_mouseButtonPressed;
        }

        CursorMode toCursorMode(RGFW_bool hidden, RGFW_bool disabled)
        {
            if (disabled)
            {
                return CursorMode::Disabled;
            }
            if (hidden)
            {
                return CursorMode::Hidden;
            }
            return CursorMode::Normal;
        }
    } // namespace

    KeyCode RgfwWindowBackend::mapKeyCode(int key)
    {
        switch (key)
        {
        case ' ':
            return KeyCode::Space;
        case '\'':
            return KeyCode::Apostrophe;
        case ',':
            return KeyCode::Comma;
        case '-':
            return KeyCode::Minus;
        case '.':
            return KeyCode::Period;
        case '/':
            return KeyCode::Slash;
        case '0':
            return KeyCode::Num0;
        case '1':
            return KeyCode::Num1;
        case '2':
            return KeyCode::Num2;
        case '3':
            return KeyCode::Num3;
        case '4':
            return KeyCode::Num4;
        case '5':
            return KeyCode::Num5;
        case '6':
            return KeyCode::Num6;
        case '7':
            return KeyCode::Num7;
        case '8':
            return KeyCode::Num8;
        case '9':
            return KeyCode::Num9;
        case ';':
            return KeyCode::Semicolon;
        case '=':
            return KeyCode::Equal;
        case 'a':
            return KeyCode::A;
        case 'b':
            return KeyCode::B;
        case 'c':
            return KeyCode::C;
        case 'd':
            return KeyCode::D;
        case 'e':
            return KeyCode::E;
        case 'f':
            return KeyCode::F;
        case 'g':
            return KeyCode::G;
        case 'h':
            return KeyCode::H;
        case 'i':
            return KeyCode::I;
        case 'j':
            return KeyCode::J;
        case 'k':
            return KeyCode::K;
        case 'l':
            return KeyCode::L;
        case 'm':
            return KeyCode::M;
        case 'n':
            return KeyCode::N;
        case 'o':
            return KeyCode::O;
        case 'p':
            return KeyCode::P;
        case 'q':
            return KeyCode::Q;
        case 'r':
            return KeyCode::R;
        case 's':
            return KeyCode::S;
        case 't':
            return KeyCode::T;
        case 'u':
            return KeyCode::U;
        case 'v':
            return KeyCode::V;
        case 'w':
            return KeyCode::W;
        case 'x':
            return KeyCode::X;
        case 'y':
            return KeyCode::Y;
        case 'z':
            return KeyCode::Z;
        case '[':
            return KeyCode::LeftBracket;
        case '\\':
            return KeyCode::Backslash;
        case ']':
            return KeyCode::RightBracket;
        case '`':
            return KeyCode::GraveAccent;
        case 27:
            return KeyCode::Escape;
        case 13:
            return KeyCode::Enter;
        case 9:
            return KeyCode::Tab;
        case 8:
            return KeyCode::Backspace;
        case 127:
            return KeyCode::Delete;
        default:
            return KeyCode::Unknown;
        }
    }

    MouseButton RgfwWindowBackend::mapMouseButton(int button)
    {
        switch (static_cast<RGFW_mouseButton>(button))
        {
        case RGFW_mouseLeft:
            return MouseButton::Left;
        case RGFW_mouseRight:
            return MouseButton::Right;
        case RGFW_mouseMiddle:
            return MouseButton::Middle;
        default:
            return MouseButton::Left;
        }
    }

    RgfwWindowBackend::RgfwWindowBackend(const WindowConfig &config, InputState &input, WindowEventCallback eventCallback)
        : m_input(input), m_eventCallback(std::move(eventCallback))
    {
        const auto flags = static_cast<RGFW_windowFlags>(config.resizable ? RGFW_windowCenter : (RGFW_windowCenter | RGFW_windowNoResize));
        m_handle = RGFW_createWindow(config.title.c_str(), 100, 100, config.width, config.height, flags);
        if (!m_handle)
        {
            throw std::runtime_error("Failed to create RGFW window");
        }

        RGFW_window_setUserPtr(m_handle, this);
        RGFW_window_setEnabledEvents(m_handle, RGFW_allEventFlags);

        int width = config.width;
        int height = config.height;
        RGFW_window_getSize(m_handle, &width, &height);
        m_size = {width, height};
        m_framebufferSize = m_size;
    }

    RgfwWindowBackend::~RgfwWindowBackend()
    {
        if (m_handle)
        {
            RGFW_window_close(m_handle);
            m_handle = nullptr;
        }
    }

    void RgfwWindowBackend::pollEvents()
    {
        RGFW_pollEvents();

        RGFW_event event{};
        while (RGFW_window_checkQueuedEvent(m_handle, &event))
        {
            switch (event.type)
            {
            case 1:
                m_input.onKeyEvent(mapKeyCode(static_cast<int>(event.key.value)), true);
                break;
            case 2:
                m_input.onKeyEvent(mapKeyCode(static_cast<int>(event.key.value)), false);
                break;
            case 4:
                m_input.onMouseButtonEvent(mapMouseButton(static_cast<int>(event.button.value)), true);
                break;
            case 5:
                m_input.onMouseButtonEvent(mapMouseButton(static_cast<int>(event.button.value)), false);
                break;
            case 7:
                m_input.onMouseMove(event.mouse.x, event.mouse.y);
                break;
            case 6:
                m_input.onMouseScroll(event.scroll.x, event.scroll.y);
                break;
            case 13:
                m_input.setFocused(true);
                dispatchEvent(WindowEvent{.type = WindowEventType::FocusChanged, .focused = true});
                break;
            case 14:
                m_input.setFocused(false);
                dispatchEvent(WindowEvent{.type = WindowEventType::FocusChanged, .focused = false});
                break;
            case 12:
            {
                int width = 0;
                int height = 0;
                RGFW_window_getSize(m_handle, &width, &height);
                m_size = {width, height};
                m_framebufferSize = m_size;
                dispatchEvent(WindowEvent{.type = WindowEventType::Resized, .size = m_size});
                dispatchEvent(WindowEvent{.type = WindowEventType::FramebufferResized, .size = m_framebufferSize});
                break;
            }
            case 16:
                dispatchEvent(WindowEvent{.type = WindowEventType::CloseRequested});
                break;
            default:
                break;
            }
        }

        int width = 0;
        int height = 0;
        if (RGFW_window_getSize(m_handle, &width, &height) == RGFW_TRUE)
        {
            m_size = {width, height};
            m_framebufferSize = m_size;
        }
    }

    void RgfwWindowBackend::swapBuffers()
    {
    }

    bool RgfwWindowBackend::shouldClose() const
    {
        return RGFW_window_shouldClose(m_handle) == RGFW_TRUE;
    }

    void RgfwWindowBackend::setShouldClose(bool close)
    {
        if (close)
        {
            RGFW_window_close(m_handle);
        }
    }

    void RgfwWindowBackend::setTitle(const std::string &title)
    {
        RGFW_window_setName(m_handle, title.c_str());
    }

    void RgfwWindowBackend::setVSync(bool enabled)
    {
        (void)enabled;
    }

    glm::ivec2 RgfwWindowBackend::getSize() const
    {
        return m_size;
    }

    glm::ivec2 RgfwWindowBackend::getFramebufferSize() const
    {
        return m_framebufferSize;
    }

    float RgfwWindowBackend::getAspectRatio() const
    {
        return m_framebufferSize.y == 0 ? 0.0f : static_cast<float>(m_framebufferSize.x) / static_cast<float>(m_framebufferSize.y);
    }

    void *RgfwWindowBackend::getNativeHandle() const
    {
#if defined(_WIN32)
        return RGFW_window_getHWND(m_handle);
#else
        return m_handle;
#endif
    }

    void RgfwWindowBackend::setCursorMode(CursorMode mode)
    {
        m_cursorMode = mode;
    }

    CursorMode RgfwWindowBackend::getCursorMode() const
    {
        return m_cursorMode;
    }

    const char *RgfwWindowBackend::getBackendName() const
    {
        return "RGFW";
    }

    GraphicsApi RgfwWindowBackend::getGraphicsApi() const
    {
        return GraphicsApi::Bgfx;
    }

    void RgfwWindowBackend::dispatchEvent(const WindowEvent &event) const
    {
        if (m_eventCallback)
        {
            m_eventCallback(event);
        }
    }

    std::unique_ptr<WindowBackend> createDefaultWindowBackend(
        const WindowConfig &config,
        InputState &input,
        WindowEventCallback eventCallback)
    {
        return std::make_unique<RgfwWindowBackend>(config, input, std::move(eventCallback));
    }

} // namespace Zenith
