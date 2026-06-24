#include "scene/scene_manager.hpp"

#include "scene/scene.hpp"
#include "scene/system_registry.hpp"

namespace Zenith
{
    bool SceneManager::SceneLoadToken::ready() const
    {
        std::scoped_lock lock(m_mutex);
        return m_ready;
    }

    std::unique_ptr<Scene> SceneManager::SceneLoadToken::takeScene()
    {
        std::scoped_lock lock(m_mutex);
        m_ready = false;
        return std::move(m_scene);
    }

    void SceneManager::SceneLoadToken::setScene(std::unique_ptr<Scene> scene)
    {
        std::scoped_lock lock(m_mutex);
        m_scene = std::move(scene);
        m_ready = true;
    }

    SceneManager::SceneLoadJob::SceneLoadJob(std::function<std::unique_ptr<Scene>()> factory)
    {
        start(std::move(factory));
    }

    SceneManager::SceneLoadJob::~SceneLoadJob()
    {
        stop();
    }

    SceneManager::SceneLoadJob::SceneLoadJob(SceneLoadJob &&other) noexcept
        : m_token(std::move(other.m_token)), m_worker(std::move(other.m_worker))
    {
    }

    SceneManager::SceneLoadJob &SceneManager::SceneLoadJob::operator=(SceneLoadJob &&other) noexcept
    {
        if (this != &other)
        {
            stop();
            m_token = std::move(other.m_token);
            m_worker = std::move(other.m_worker);
        }
        return *this;
    }

    std::unique_ptr<Scene> SceneManager::SceneLoadJob::takeScene()
    {
        if (!m_token)
        {
            return nullptr;
        }

        wait();
        return m_token->takeScene();
    }

    void SceneManager::SceneLoadJob::wait()
    {
        if (m_worker.joinable())
        {
            m_worker.join();
        }
    }

    void SceneManager::SceneLoadJob::start(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_token = std::make_unique<SceneLoadToken>();
        m_worker = std::thread([token = m_token.get(), factory = std::move(factory)]() mutable
                               { token->setScene(factory ? factory() : nullptr); });
    }

    void SceneManager::SceneLoadJob::stop()
    {
        wait();
    }

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

    SceneManager::SceneLoadJob SceneManager::prepareSceneAsync(std::function<std::unique_ptr<Scene>()> factory)
    {
        return SceneLoadJob(std::move(factory));
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
        if (m_sceneStack.empty())
        {
            return false;
        }

        if (m_activeScene)
        {
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

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
