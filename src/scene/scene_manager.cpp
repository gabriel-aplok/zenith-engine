#include "scene/scene_manager.hpp"

#include "scene/scene.hpp"
#include "scene/system_registry.hpp"

namespace Zenith
{
    SceneManager::SceneManager()
        : m_systems(std::make_unique<SystemRegistry>())
    {
    }

    SceneManager::~SceneManager() = default;

    Scene *SceneManager::currentScene()
    {
        return m_activeScene.get();
    }

    const Scene *SceneManager::currentScene() const
    {
        return m_activeScene.get();
    }

    void SceneManager::setScene(std::unique_ptr<Scene> scene)
    {
        m_pendingScene = std::move(scene);
        commitPendingScene();
    }

    void SceneManager::addSystem(std::unique_ptr<System> system)
    {
        m_systems->addSystem(std::move(system));
    }

    void SceneManager::requestScene(std::unique_ptr<Scene> scene)
    {
        m_pendingScene = std::move(scene);
    }

    void SceneManager::commitPendingScene()
    {
        if (!m_pendingScene)
        {
            return;
        }

        if (m_activeScene)
        {
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        m_pendingScene->onLoad();
        m_activeScene = std::move(m_pendingScene);
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
    }

    void SceneManager::update(float deltaTime)
    {
        commitPendingScene();

        if (!m_activeScene)
        {
            return;
        }

        m_activeScene->flushCommands();
        m_systems->update(*m_activeScene, deltaTime);
        m_activeScene->flushCommands();
        commitPendingScene();
    }

    void SceneManager::render(RenderFrame &frame)
    {
        commitPendingScene();

        if (!m_activeScene)
        {
            return;
        }

        m_activeScene->flushCommands();
        m_systems->render(*m_activeScene, frame);
        m_activeScene->flushCommands();
        commitPendingScene();
    }

    void SceneManager::clear()
    {
        m_pendingScene.reset();

        if (m_activeScene)
        {
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        m_systems->clear();
    }
} // namespace Zenith
