#pragma once

#include <chrono>
#include <memory>

#include "engine/window.hpp"

namespace Zenith
{
    struct ApplicationConfig
    {
        WindowConfig window{};
    };

    class Application
    {
    public:
        explicit Application(const ApplicationConfig &config = {});
        virtual ~Application() = default;

        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        void run();
        void requestQuit();

    protected:
        virtual void onInit() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}
        virtual void onShutdown() {}
        virtual void onWindowEvent(const WindowEvent &event) {}

        Window &getWindow() { return *m_window; }
        const Window &getWindow() const { return *m_window; }
        InputState &getInput() { return m_window->getInput(); }
        const InputState &getInput() const { return m_window->getInput(); }

    private:
        std::unique_ptr<Window> m_window;
        bool m_running = true;

        using Clock = std::chrono::steady_clock;
        std::chrono::time_point<Clock> m_lastTime;
    };
} // namespace Zenith
