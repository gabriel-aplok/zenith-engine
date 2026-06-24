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
        if (!scene)
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

        while (!m_sceneStack.empty())
        {
            m_sceneStack.back()->onExit();
            m_sceneStack.back()->onUnload();
            m_sceneStack.pop_back();
        }

        scene->onLoad();
        m_activeScene = std::move(scene);
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
    }

    void SceneManager::addSystem(std::unique_ptr<System> system)
    {
        m_systems->addSystem(std::move(system));
    }

    void SceneManager::pushScene(std::unique_ptr<Scene> scene)
    {
        if (!scene)
        {
            return;
        }

        if (m_activeScene)
        {
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_sceneStack.push_back(std::move(m_activeScene));
        }

        scene->onLoad();
        m_activeScene = std::move(scene);
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
    }

    bool SceneManager::popScene()
    {
        if (!m_activeScene || m_sceneStack.empty())
        {
            return false;
        }

        m_systems->bindScene(nullptr);
        m_activeScene->onExit();
        m_activeScene->onUnload();
        m_activeScene.reset();

        m_activeScene = std::move(m_sceneStack.back());
        m_sceneStack.pop_back();
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
        return true;
    }

    void SceneManager::update(float deltaTime)
    {
        if (!m_activeScene)
        {
            return;
        }

        m_activeScene->flushCommands();
        m_systems->update(*m_activeScene, deltaTime);
        m_activeScene->flushCommands();
    }

    void SceneManager::render(RenderFrame &frame)
    {
        if (!m_activeScene)
        {
            return;
        }

        m_activeScene->flushCommands();
        m_systems->render(*m_activeScene, frame);
        m_activeScene->flushCommands();
    }

    void SceneManager::clear()
    {
        if (m_activeScene)
        {
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        while (!m_sceneStack.empty())
        {
            m_sceneStack.back()->onExit();
            m_sceneStack.back()->onUnload();
            m_sceneStack.pop_back();
        }

        m_systems->clear();
    }
} // namespace Zenith
