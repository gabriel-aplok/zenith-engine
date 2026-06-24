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
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
        m_transitionState = TransitionState::Loading;
        commitScene();
    }

    void SceneManager::prepareScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingScene = std::move(scene);
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::prepareSceneFactory(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_pendingScene.reset();
        m_pendingFactory = std::move(factory);
        m_transitionState = TransitionState::Loading;
    }

    void SceneManager::addSystem(std::unique_ptr<System> system)
    {
        m_systems->addSystem(std::move(system));
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
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        m_transitionState = TransitionState::Entering;
        m_activeScene = std::move(m_pendingScene);
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
        m_transitionState = TransitionState::Idle;
        return true;
    }

    void SceneManager::update(float deltaTime)
    {
        if (m_transitionState == TransitionState::PendingCommit && m_pendingScene)
        {
            commitScene();
        }
        if (m_activeScene)
        {
            m_activeScene->flushCommands();
            m_systems->update(*m_activeScene, deltaTime);
            m_activeScene->flushCommands();
        }
    }

    void SceneManager::render(RenderFrame &frame)
    {
        if (m_activeScene)
        {
            m_activeScene->flushCommands();
            m_systems->render(*m_activeScene, frame);
            m_activeScene->flushCommands();
        }
    }

    void SceneManager::clear()
    {
        m_pendingFactory = {};
        m_pendingScene.reset();
        if (m_activeScene)
        {
            m_transitionState = TransitionState::Exiting;
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }
        if (m_systems)
        {
            m_systems->clear();
        }
        m_transitionState = TransitionState::Idle;
    }
} // namespace Zenith
