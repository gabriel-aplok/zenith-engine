#pragma once

#include "engine/window.hpp"
#include <memory>
#include <chrono>

namespace Zenith {

    class Application {
    public:
        Application();
        virtual ~Application() = default;

        // Delete copy
        Application(const Application &) = delete;
        Application &operator=(const Application &) = delete;

        void run();

    protected:
        virtual void onInit() {}
        virtual void onUpdate(float deltaTime) {}
        virtual void onRender() {}
        virtual void onGui() {}
        virtual void onShutdown() {}

        Window &getWindow() { return *m_window; }

    private:
        std::unique_ptr<Window> m_window;
        bool m_running = true;

        using Clock = std::chrono::high_resolution_clock;
        std::chrono::time_point<Clock> m_lastTime;
    };

} // namespace Zenith