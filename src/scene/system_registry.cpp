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

        if (m_activeScene)
        {
            system->onAdd(*m_activeScene);
            system->onStart(*m_activeScene);
        }

        m_systems.emplace_back(std::move(system));
    }

    void SystemRegistry::bindScene(Scene *scene)
    {
        if (m_activeScene == scene)
        {
            return;
        }

        if (m_activeScene)
        {
            for (auto &system : m_systems)
            {
                system->onStop(*m_activeScene);
                system->onRemove(*m_activeScene);
            }
        }

        m_activeScene = scene;
        if (m_activeScene)
        {
            for (auto &system : m_systems)
            {
                system->onAdd(*m_activeScene);
                system->onStart(*m_activeScene);
            }
        }
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
        if (m_activeScene)
        {
            for (auto &system : m_systems)
            {
                system->onStop(*m_activeScene);
                system->onRemove(*m_activeScene);
            }
        }
        m_systems.clear();
        m_activeScene = nullptr;
    }
} // namespace Zenith
