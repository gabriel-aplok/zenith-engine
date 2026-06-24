#include "scene/scene_manager.hpp"

#include "scene/scene.hpp"

namespace Zenith
{
    Scene *SceneManager::currentScene()
    {
        applyPendingScene();
        return m_activeScene.get();
    }

    const Scene *SceneManager::currentScene() const
    {
        return const_cast<SceneManager *>(this)->currentScene();
    }

    void SceneManager::setScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
        applyPendingScene();
    }

    void SceneManager::requestScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
    }

    void SceneManager::requestSceneFactory(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_pendingScene.reset();
        m_pendingFactory = std::move(factory);
    }

    void SceneManager::update()
    {
        applyPendingScene();
    }

    void SceneManager::clear()
    {
        m_pendingFactory = {};
        m_pendingScene.reset();
        if (m_activeScene)
        {
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }
    }

    void SceneManager::applyPendingScene()
    {
        if (!m_pendingScene && !m_pendingFactory)
        {
            return;
        }

        if (m_activeScene)
        {
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        if (m_pendingFactory)
        {
            m_pendingScene = m_pendingFactory();
            m_pendingFactory = {};
        }

        m_activeScene = std::move(m_pendingScene);
        if (m_activeScene)
        {
            m_activeScene->onEnter();
        }
    }
} // namespace Zenith
