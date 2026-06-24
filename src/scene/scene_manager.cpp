#include "scene/scene_manager.hpp"

#include "scene/scene.hpp"

namespace Zenith
{
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
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
        m_transitionState = TransitionState::Loading;
        commitScene();
    }

    void SceneManager::requestScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::requestSceneFactory(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_pendingScene.reset();
        m_pendingFactory = std::move(factory);
        m_transitionState = TransitionState::Loading;
    }

    bool SceneManager::commitScene()
    {
        if (m_pendingFactory)
        {
            m_transitionState = TransitionState::Loading;
            m_pendingScene = m_pendingFactory();
            m_pendingFactory = {};
        }

        if (!m_pendingScene)
        {
            return false;
        }

        m_pendingScene->onLoad();

        if (m_activeScene)
        {
            m_transitionState = TransitionState::Exiting;
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        m_transitionState = TransitionState::Entering;
        m_activeScene = std::move(m_pendingScene);
        m_activeScene->onEnter();
        m_transitionState = TransitionState::Idle;
        return true;
    }

    void SceneManager::update()
    {
        if (m_transitionState == TransitionState::PendingCommit && m_pendingScene)
        {
            commitScene();
        }
    }

    void SceneManager::clear()
    {
        m_pendingFactory = {};
        m_pendingScene.reset();
        if (m_activeScene)
        {
            m_transitionState = TransitionState::Exiting;
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }
        m_transitionState = TransitionState::Idle;
    }
} // namespace Zenith
