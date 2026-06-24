#pragma once

#include <functional>
#include <memory>

namespace Zenith
{
    class Scene;

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

        SceneManager() = default;
        ~SceneManager() = default;

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        bool hasScene() const { return static_cast<bool>(m_activeScene); }
        TransitionState transitionState() const { return m_transitionState; }
        bool hasPendingScene() const { return static_cast<bool>(m_pendingScene) || static_cast<bool>(m_pendingFactory); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
        void requestScene(std::unique_ptr<Scene> scene);
        void requestSceneFactory(std::function<std::unique_ptr<Scene>()> factory);
        bool canCommitScene() const { return hasPendingScene(); }
        bool commitScene();
        void update();
        void clear();

    private:
        TransitionState m_transitionState = TransitionState::Idle;
        std::unique_ptr<Scene> m_activeScene;
        std::unique_ptr<Scene> m_pendingScene;
        std::function<std::unique_ptr<Scene>()> m_pendingFactory;
    };
} // namespace Zenith
