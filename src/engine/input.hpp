#pragma once

#include <array>
#include <cstddef>

#include "math/math.hpp"

namespace Zenith
{
    enum class KeyCode : std::size_t
    {
        Unknown = 0,
        Space,
        Apostrophe,
        Comma,
        Minus,
        Period,
        Slash,
        Num0,
        Num1,
        Num2,
        Num3,
        Num4,
        Num5,
        Num6,
        Num7,
        Num8,
        Num9,
        Semicolon,
        Equal,
        A,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        LeftBracket,
        Backslash,
        RightBracket,
        GraveAccent,
        Escape,
        Enter,
        Tab,
        Backspace,
        Insert,
        Delete,
        Right,
        Left,
        Down,
        Up,
        PageUp,
        PageDown,
        Home,
        End,
        CapsLock,
        ScrollLock,
        NumLock,
        PrintScreen,
        Pause,
        F1,
        F2,
        F3,
        F4,
        F5,
        F6,
        F7,
        F8,
        F9,
        F10,
        F11,
        F12,
        LeftShift,
        LeftControl,
        LeftAlt,
        RightShift,
        RightControl,
        RightAlt,
        Count
    };

    enum class MouseButton : std::size_t
    {
        Left = 0,
        Right,
        Middle,
        Button4,
        Button5,
        Count
    };

    class InputState
    {
    public:
        enum class ButtonState : unsigned char
        {
            Up = 0,
            Pressed,
            Held,
            Released
        };

        void beginFrame();
        void clear();

        void onKeyEvent(KeyCode key, bool pressed);
        void onMouseButtonEvent(MouseButton button, bool pressed);
        void onMouseMove(double x, double y);
        void onMouseScroll(double xOffset, double yOffset);
        void setFocused(bool focused);

        bool isKeyPressed(KeyCode key) const;
        bool isKeyReleased(KeyCode key) const;
        bool isKeyHeld(KeyCode key) const;
        bool isKeyDown(KeyCode key) const;

        bool isMouseButtonPressed(MouseButton button) const;
        bool isMouseButtonReleased(MouseButton button) const;
        bool isMouseButtonHeld(MouseButton button) const;
        bool isMouseButtonDown(MouseButton button) const;

        DVector2 getMousePosition() const;
        DVector2 getMouseDelta() const;
        DVector2 getScrollDelta() const;

        bool isFocused() const;

    private:
        template <std::size_t N>
        static void advanceButtonStates(std::array<ButtonState, N> &states);

        template <std::size_t N>
        static bool isDown(const std::array<ButtonState, N> &states, std::size_t index);

        std::array<ButtonState, static_cast<std::size_t>(KeyCode::Count)> m_keyStates{};
        std::array<ButtonState, static_cast<std::size_t>(MouseButton::Count)> m_mouseButtonStates{};
        DVector2 m_mousePosition{0.0, 0.0};
        DVector2 m_mouseDelta{0.0, 0.0};
        DVector2 m_scrollDelta{0.0, 0.0};
        bool m_hasMousePosition = false;
        bool m_focused = true;
    };

} // namespace Zenith
