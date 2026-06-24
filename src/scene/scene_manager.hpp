#pragma once

#include <functional>
#include <memory>

namespace Zenith
{
    class Scene;
    class System;
    class SystemRegistry;
    class RenderFrame;

    class SceneManager
    {
    public:
        enum class TransitionState
        {
            Idle,
            Loading,
            PendingCommit,
            Exiting,
            Entering,
        };

        SceneManager();
        ~SceneManager();

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        bool hasScene() const { return static_cast<bool>(m_activeScene); }
        TransitionState transitionState() const { return m_transitionState; }
        bool hasPendingScene() const { return static_cast<bool>(m_pendingScene) || static_cast<bool>(m_pendingFactory); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
        void prepareScene(std::unique_ptr<Scene> scene);
        void prepareSceneFactory(std::function<std::unique_ptr<Scene>()> factory);
        void addSystem(std::unique_ptr<System> system);
        bool commitScene();
        void update(float deltaTime);
        void render(RenderFrame &frame);
        void clear();

    private:
        TransitionState m_transitionState = TransitionState::Idle;
        std::unique_ptr<Scene> m_activeScene;
        std::unique_ptr<Scene> m_pendingScene;
        std::function<std::unique_ptr<Scene>()> m_pendingFactory;
        std::unique_ptr<SystemRegistry> m_systems;
    };
} // namespace Zenith
