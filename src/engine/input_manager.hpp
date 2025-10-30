#pragma once
#include <glm/glm.hpp>
#include <unordered_set>

namespace Zenith {

    class InputManager {
    public:
        static InputManager &get();

        void update();

        bool isKeyPressed(int key) const;
        bool isKeyJustPressed(int key) const;
        bool isMouseButtonPressed(int button) const;
        glm::dvec2 getMouseDelta() const;
        glm::dvec2 getMousePosition() const;

        // Internal callbacks
        void onKey(int key, int action);
        void onMouseButton(int button, int action);
        void onCursorPos(double xpos, double ypos);

    private:
        InputManager() = default;

        std::unordered_set<int> m_keysDown;
        std::unordered_set<int> m_keysJustPressed;
        std::unordered_set<int> m_mouseButtonsDown;

        glm::dvec2 m_mousePos{0}, m_lastMousePos{0};
    };

} // namespace Zenith