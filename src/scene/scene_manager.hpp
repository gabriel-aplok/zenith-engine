#pragma once

#include <functional>
#include <memory>
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
        SceneManager();
        ~SceneManager();

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        bool hasScene() const { return static_cast<bool>(m_activeScene); }
        bool hasSceneStack() const { return !m_sceneStack.empty(); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
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
