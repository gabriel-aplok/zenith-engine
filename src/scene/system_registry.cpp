#include "scene/system_registry.hpp"

#include "scene/scene.hpp"

namespace Zenith
{
    void SystemRegistry::addSystem(std::unique_ptr<System> system)
    {
        if (!system)
        {
            return;
        }

        if (m_boundScene)
        {
            system->onAdd(*m_boundScene);
            system->onStart(*m_boundScene);
        }

        m_systems.emplace_back(std::move(system));
    }

    void SystemRegistry::bindScene(Scene *scene)
    {
        if (m_activeScene == scene)
        {
            return;
        }

        stopActiveScene();
        m_boundScene = scene;
        startActiveScene();
    }

    void SystemRegistry::update(Scene &scene, float deltaTime)
    {
        bindScene(&scene);
        for (auto &system : m_systems)
        {
            system->preUpdate(scene, deltaTime);
            system->update(scene, deltaTime);
            system->postUpdate(scene, deltaTime);
        }
    }

    void SystemRegistry::render(Scene &scene, RenderFrame &frame)
    {
        bindScene(&scene);
        for (auto &system : m_systems)
        {
            system->preRender(scene, frame);
            system->render(scene, frame);
            system->postRender(scene, frame);
        }
    }

    void SystemRegistry::clear()
    {
        stopActiveScene();
        for (auto &system : m_systems)
        {
            if (m_boundScene)
            {
                system->onRemove(*m_boundScene);
            }
        }
        m_systems.clear();
        m_boundScene = nullptr;
    }

    void SystemRegistry::stopActiveScene()
    {
        if (!m_activeScene)
        {
            return;
        }

        for (auto &system : m_systems)
        {
            system->onStop(*m_activeScene);
        }
        m_activeScene = nullptr;
    }

    void SystemRegistry::startActiveScene()
    {
        if (!m_boundScene)
        {
            return;
        }

        for (auto &system : m_systems)
        {
            system->onAdd(*m_boundScene);
            system->onStart(*m_boundScene);
        }
        m_activeScene = m_boundScene;
    }
} // namespace Zenith
