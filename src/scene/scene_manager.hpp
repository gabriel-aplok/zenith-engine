#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

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
            std::unique_ptr<Scene> takeScene();
            void wait();

        private:
            void start(std::function<std::unique_ptr<Scene>()> factory);
            void stop();

            std::unique_ptr<SceneLoadToken> m_token;
            std::thread m_worker;
        };

        SceneManager();
        ~SceneManager();

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        bool hasScene() const { return static_cast<bool>(m_activeScene); }
        bool hasSceneStack() const { return !m_sceneStack.empty(); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
        SceneLoadJob prepareSceneAsync(std::function<std::unique_ptr<Scene>()> factory);
        void addSystem(std::unique_ptr<System> system);
        void pushScene(std::unique_ptr<Scene> scene);
        bool popScene();
        void update(float deltaTime);
        void render(RenderFrame &frame);
        void clear();

    private:
        std::unique_ptr<Scene> m_activeScene;
        std::unique_ptr<SystemRegistry> m_systems;
        std::vector<std::unique_ptr<Scene>> m_sceneStack;
    };
} // namespace Zenith
