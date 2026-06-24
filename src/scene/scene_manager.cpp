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
        {
            std::scoped_lock lock(m_mutex);
            m_scene = std::move(scene);
            m_ready = true;
        }
        m_readyCondition.notify_all();
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
                               {
                                   auto scene = factory ? factory() : nullptr;
                                   token->setScene(std::move(scene)); });
    }

    void SceneManager::SceneLoadJob::stop()
    {
        if (m_worker.joinable())
        {
            m_worker.join();
        }
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
        m_pendingFactory = {};
        m_pendingLoadJob.reset();
        m_pendingScene = std::move(scene);
        m_pendingPush = false;
        m_transitionState = TransitionState::Loading;
        commitScene();
    }

    void SceneManager::prepareScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingLoadJob.reset();
        m_pendingScene = std::move(scene);
        m_pendingPush = false;
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::prepareSceneFactory(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_pendingScene.reset();
        m_pendingLoadJob.reset();
        m_pendingFactory = std::move(factory);
        m_pendingPush = false;
        m_transitionState = TransitionState::Loading;
    }

    SceneManager::SceneLoadJob SceneManager::prepareSceneAsync(std::function<std::unique_ptr<Scene>()> factory)
    {
        return SceneLoadJob(std::move(factory));
    }

    void SceneManager::acceptPreparedScene(SceneLoadJob &job)
    {
        if (!job.valid())
        {
            return;
        }

        job.wait();
        m_pendingScene = job.token()->takeScene();
        m_pendingFactory = {};
        m_pendingLoadJob.reset();
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::pushPreparedScene(SceneLoadJob &job)
    {
        if (!job.valid())
        {
            return;
        }

        job.wait();
        m_pendingScene = job.token()->takeScene();
        m_pendingFactory = {};
        m_pendingLoadJob.reset();
        m_pendingPush = true;
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::pushScene(std::unique_ptr<Scene> scene)
    {
        m_pendingFactory = {};
        m_pendingLoadJob.reset();
        m_pendingScene = std::move(scene);
        m_pendingPush = true;
        m_transitionState = TransitionState::PendingCommit;
    }

    void SceneManager::pushSceneFactory(std::function<std::unique_ptr<Scene>()> factory)
    {
        m_pendingScene.reset();
        m_pendingLoadJob.reset();
        m_pendingFactory = std::move(factory);
        m_pendingPush = true;
        m_transitionState = TransitionState::Loading;
    }

    bool SceneManager::popScene()
    {
        if (m_sceneStack.empty())
        {
            return false;
        }

        if (m_activeScene)
        {
            m_transitionState = TransitionState::Exiting;
            m_systems->bindScene(nullptr);
            m_activeScene->onExit();
            m_activeScene->onUnload();
            m_activeScene.reset();
        }

        m_activeScene = std::move(m_sceneStack.back());
        m_sceneStack.pop_back();
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
        m_transitionState = TransitionState::Idle;
        return true;
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

        if (m_pendingLoadJob && m_pendingLoadJob->ready())
        {
            m_pendingScene = m_pendingLoadJob->token()->takeScene();
            m_pendingLoadJob.reset();
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
            if (!m_pendingPush)
            {
                m_activeScene->onUnload();
                m_activeScene.reset();
                m_sceneStack.clear();
            }
            else
            {
                m_sceneStack.push_back(std::move(m_activeScene));
            }
        }

        m_transitionState = TransitionState::Entering;
        m_activeScene = std::move(m_pendingScene);
        m_activeScene->onEnter();
        m_systems->bindScene(m_activeScene.get());
        if (!m_pendingPush)
        {
            m_sceneStack.clear();
        }
        m_transitionState = TransitionState::Idle;
        m_pendingPush = false;
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
        m_pendingLoadJob.reset();
        m_pendingScene.reset();
        m_pendingPush = false;
        if (m_activeScene)
        {
            m_transitionState = TransitionState::Exiting;
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
        m_sceneStack.clear();
        if (m_systems)
        {
            m_systems->clear();
        }
        m_transitionState = TransitionState::Idle;
    }
} // namespace Zenith
