#include "engine/input.hpp"

namespace Zenith
{
    namespace
    {
        std::size_t toIndex(KeyCode key)
        {
            return static_cast<std::size_t>(key);
        }

        std::size_t toIndex(MouseButton button)
        {
            return static_cast<std::size_t>(button);
        }

    } // namespace

    void InputState::beginFrame()
    {
        advanceButtonStates(m_keyStates);
        advanceButtonStates(m_mouseButtonStates);
        m_mouseDelta = DVector2(0.0, 0.0);
        m_scrollDelta = DVector2(0.0, 0.0);
    }

    void InputState::clear()
    {
        m_keyStates.fill(ButtonState::Up);
        m_mouseButtonStates.fill(ButtonState::Up);
        m_mouseDelta = DVector2(0.0, 0.0);
        m_scrollDelta = DVector2(0.0, 0.0);
    }

    void InputState::onKeyEvent(KeyCode key, bool pressed)
    {
        if (key == KeyCode::Unknown)
            return;

        auto &state = m_keyStates[toIndex(key)];
        if (pressed)
            state = isDown(m_keyStates, toIndex(key)) ? ButtonState::Held : ButtonState::Pressed;
        else
            state = ButtonState::Released;
    }

    void InputState::onMouseButtonEvent(MouseButton button, bool pressed)
    {
        auto &state = m_mouseButtonStates[toIndex(button)];
        if (pressed)
            state = isDown(m_mouseButtonStates, toIndex(button)) ? ButtonState::Held : ButtonState::Pressed;
        else
            state = ButtonState::Released;
    }

    void InputState::onMouseMove(double x, double y)
    {
        const DVector2 newPosition{x, y};
        if (m_hasMousePosition)
            m_mouseDelta += newPosition - m_mousePosition;

        m_mousePosition = newPosition;
        m_hasMousePosition = true;
    }

    void InputState::onMouseScroll(double xOffset, double yOffset)
    {
        m_scrollDelta += DVector2(xOffset, yOffset);
    }

    void InputState::setFocused(bool focused)
    {
        m_focused = focused;
        if (!focused)
            clear();
    }

    bool InputState::isKeyPressed(KeyCode key) const
    {
        return m_keyStates[toIndex(key)] == ButtonState::Pressed;
    }

    bool InputState::isKeyReleased(KeyCode key) const
    {
        return m_keyStates[toIndex(key)] == ButtonState::Released;
    }

    bool InputState::isKeyHeld(KeyCode key) const
    {
        return m_keyStates[toIndex(key)] == ButtonState::Held;
    }

    bool InputState::isKeyDown(KeyCode key) const
    {
        return isDown(m_keyStates, toIndex(key));
    }

    bool InputState::isMouseButtonPressed(MouseButton button) const
    {
        return m_mouseButtonStates[toIndex(button)] == ButtonState::Pressed;
    }

    bool InputState::isMouseButtonReleased(MouseButton button) const
    {
        return m_mouseButtonStates[toIndex(button)] == ButtonState::Released;
    }

    bool InputState::isMouseButtonHeld(MouseButton button) const
    {
        return m_mouseButtonStates[toIndex(button)] == ButtonState::Held;
    }

    bool InputState::isMouseButtonDown(MouseButton button) const
    {
        return isDown(m_mouseButtonStates, toIndex(button));
    }

    DVector2 InputState::getMousePosition() const
    {
        return m_mousePosition;
    }

    DVector2 InputState::getMouseDelta() const
    {
        return m_mouseDelta;
    }

    DVector2 InputState::getScrollDelta() const
    {
        return m_scrollDelta;
    }

    bool InputState::isFocused() const
    {
        return m_focused;
    }

    template <std::size_t N>
    void InputState::advanceButtonStates(std::array<ButtonState, N> &states)
    {
        for (auto &state : states)
        {
            if (state == ButtonState::Pressed)
                state = ButtonState::Held;
            else if (state == ButtonState::Released)
                state = ButtonState::Up;
        }
    }

    template <std::size_t N>
    bool InputState::isDown(const std::array<ButtonState, N> &states, std::size_t index)
    {
        const auto state = states[index];
        return state == ButtonState::Pressed || state == ButtonState::Held;
    }

} // namespace Zenith
