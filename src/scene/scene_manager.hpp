#pragma once

#include <functional>
#include <memory>

namespace Zenith
{
    class Scene;

    class SceneManager
    {
    public:
        SceneManager() = default;
        ~SceneManager() = default;

        SceneManager(const SceneManager &) = delete;
        SceneManager &operator=(const SceneManager &) = delete;

        bool hasScene() const { return static_cast<bool>(m_activeScene); }
        Scene *currentScene();
        const Scene *currentScene() const;

        void setScene(std::unique_ptr<Scene> scene);
        void requestScene(std::unique_ptr<Scene> scene);
        void requestSceneFactory(std::function<std::unique_ptr<Scene>()> factory);
        void update();
        void clear();

    private:
        void applyPendingScene();

        std::unique_ptr<Scene> m_activeScene;
        std::unique_ptr<Scene> m_pendingScene;
        std::function<std::unique_ptr<Scene>()> m_pendingFactory;
    };
} // namespace Zenith
