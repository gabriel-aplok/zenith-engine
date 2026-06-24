#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace Zenith
{
    class Scene;
    class System;
    class SystemRegistry;
    class RenderFrame;

    class SceneManager
    {
    public:
        class SceneLoadToken
        {
        public:
            SceneLoadToken() = default;
            bool ready() const;
            std::unique_ptr<Scene> takeScene();

        private:
            friend class SceneLoadJob;
            friend class SceneManager;

            void setScene(std::unique_ptr<Scene> scene);

            mutable std::mutex m_mutex;
            mutable std::condition_variable m_readyCondition;
            std::unique_ptr<Scene> m_scene;
            bool m_ready = false;
        };

        class SceneLoadJob
        {
        public:
            SceneLoadJob() = default;
            explicit SceneLoadJob(std::function<std::unique_ptr<Scene>()> factory);
            ~SceneLoadJob();

            SceneLoadJob(const SceneLoadJob &) = delete;
            SceneLoadJob &operator=(const SceneLoadJob &) = delete;

            SceneLoadJob(SceneLoadJob &&other) noexcept;
            SceneLoadJob &operator=(SceneLoadJob &&other) noexcept;

            bool valid() const { return static_cast<bool>(m_token); }
            bool ready() const { return m_token && m_token->ready(); }
            SceneLoadToken *token() const { return m_token.get(); }
            void wait();

        private:
            void start(std::function<std::unique_ptr<Scene>()> factory);
            void stop();

            std::unique_ptr<SceneLoadToken> m_token;
            std::thread m_worker;
        };

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
        bool hasPendingScene() const { return static_cast<bool>(m_pendingScene) || static_cast<bool>(m_pendingFactory) || static_cast<bool>(m_pendingLoadJob); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
        void prepareScene(std::unique_ptr<Scene> scene);
        void prepareSceneFactory(std::function<std::unique_ptr<Scene>()> factory);
        SceneLoadJob prepareSceneAsync(std::function<std::unique_ptr<Scene>()> factory);
        void acceptPreparedScene(SceneLoadJob &job);
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
        std::unique_ptr<SceneLoadJob> m_pendingLoadJob;
        std::unique_ptr<SystemRegistry> m_systems;
    };
} // namespace Zenith
