#include "engine/input_manager.hpp"
#include "engine/Window.hpp"
#include <GLFW/glfw3.h>

namespace Zenith {

    InputManager &InputManager::get() {
        static InputManager instance;
        return instance;
    }

    void InputManager::update() {
        m_keysJustPressed.clear();
        m_mousePos = m_lastMousePos;
    }

    bool InputManager::isKeyPressed(int key) const { return m_keysDown.count(key); }
    bool InputManager::isKeyJustPressed(int key) const { return m_keysJustPressed.count(key); }
    bool InputManager::isMouseButtonPressed(int button) const { return m_mouseButtonsDown.count(button); }
    glm::dvec2 InputManager::getMouseDelta() const { return m_mousePos - m_lastMousePos; }
    glm::dvec2 InputManager::getMousePosition() const { return m_mousePos; }

    void InputManager::onKey(int key, int action) {
        if (action == GLFW_PRESS) {
            m_keysDown.insert(key);
            m_keysJustPressed.insert(key);
        }
        else if (action == GLFW_RELEASE) {
            m_keysDown.erase(key);
        }
    }

    void InputManager::onMouseButton(int button, int action) {
        if (action == GLFW_PRESS)
            m_mouseButtonsDown.insert(button);
        else
            m_mouseButtonsDown.erase(button);
    }

    void InputManager::onCursorPos(double xpos, double ypos) {
        m_lastMousePos = m_mousePos;
        m_mousePos = glm::dvec2(xpos, ypos);
    }

} // namespace Zenith