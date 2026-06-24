#include "scene/scene.hpp"

#include <algorithm>

#include "components/script_component.hpp"

namespace Zenith
{
    void queueComponentAddition(Scene &scene, GameObject &object, std::type_index type, std::unique_ptr<Component> component)
    {
        scene.queueComponentAddition(object, type, std::move(component));
    }

    void queueComponentRemoval(Scene &scene, GameObject &object, std::type_index type)
    {
        scene.queueComponentRemoval(object, type);
    }

    GameObject &Scene::createGameObject(std::string name)
    {
        auto object = std::make_unique<GameObject>(std::move(name));
        GameObject &objectRef = *object;
        objectRef.m_scene = this;
        m_gameObjects.emplace_back(std::move(object));
        return objectRef;
    }

    void Scene::destroyGameObject(GameObject &object)
    {
        m_pendingGameObjectDestruction.push_back(&object);
    }

    void Scene::clear()
    {
        flushCommands();
        for (auto &system : m_systems)
        {
            system->onStop(*this);
            system->onRemove(*this);
        }
        m_gameObjects.clear();
        m_pendingComponentAdditions.clear();
        m_pendingGameObjectDestruction.clear();
        m_pendingComponentRemovals.clear();
    }

    void Scene::addSystem(std::unique_ptr<System> system)
    {
        system->onAdd(*this);
        system->onStart(*this);
        m_systems.emplace_back(std::move(system));
    }

    void Scene::setFramebufferSize(const glm::ivec2 &framebufferSize)
    {
        m_framebufferSize = framebufferSize;
    }

    void Scene::update(float deltaTime)
    {
        for (auto &system : m_systems)
        {
            system->preUpdate(*this, deltaTime);
            system->update(*this, deltaTime);
            system->postUpdate(*this, deltaTime);
        }
        flushCommands();
    }

    Components::Camera *Scene::findCamera()
    {
        Components::Camera *bestCamera = nullptr;
        float bestDepth = 0.0f;

        for (auto &object : m_gameObjects)
        {
            if (auto *camera = object->get_component<Components::Camera>())
            {
                if (!camera->enabled())
                {
                    continue;
                }

                if (!bestCamera || camera->depth() < bestDepth)
                {
                    bestCamera = camera;
                    bestDepth = camera->depth();
                }
            }
        }

        return bestCamera;
    }

    const Components::Camera *Scene::findCamera() const
    {
        return const_cast<Scene *>(this)->findCamera();
    }

    void Scene::render(RenderFrame &frame)
    {
        flushCommands();
        for (auto &system : m_systems)
        {
            system->preRender(*this, frame);
            system->render(*this, frame);
            system->postRender(*this, frame);
        }
        flushCommands();
    }

    void Scene::queueComponentRemoval(GameObject &object, std::type_index type)
    {
        m_pendingComponentRemovals.push_back(PendingComponentRemoval{&object, type});
    }

    void Scene::queueComponentAddition(GameObject &object, std::type_index type, std::unique_ptr<Component> component)
    {
        for (auto it = m_pendingComponentRemovals.begin(); it != m_pendingComponentRemovals.end(); ++it)
        {
            if (it->object == &object && it->type == type)
            {
                m_pendingComponentRemovals.erase(it);
                return;
            }
        }

        m_pendingComponentAdditions.push_back(PendingComponentAddition{&object, type, std::move(component)});
    }

    void Scene::flushCommands()
    {
        for (auto &addition : m_pendingComponentAdditions)
        {
            if (!addition.object || !addition.component)
            {
                continue;
            }

            auto &components = addition.object->m_components;
            auto existing = std::find_if(components.begin(), components.end(), [&addition](const auto &ptr) {
                return typeid(*ptr) == addition.type;
            });
            if (existing != components.end())
            {
                continue;
            }

            addition.component->setOwner(addition.object);
            if (auto *script = dynamic_cast<Components::ScriptComponent *>(addition.component.get()))
            {
                script->markAttached();
            }

            components.emplace_back(std::move(addition.component));
        }
        m_pendingComponentAdditions.clear();

        for (const auto &removal : m_pendingComponentRemovals)
        {
            if (!removal.object)
            {
                continue;
            }

            auto &components = removal.object->m_components;
            for (auto it = components.begin(); it != components.end(); ++it)
            {
                if (typeid(**it) == removal.type)
                {
                    if (auto *script = dynamic_cast<Components::ScriptComponent *>(it->get()))
                    {
                        script->markDetached();
                    }
                    components.erase(it);
                    break;
                }
            }
        }
        m_pendingComponentRemovals.clear();

        if (!m_pendingGameObjectDestruction.empty())
        {
            for (auto *object : m_pendingGameObjectDestruction)
            {
                if (!object)
                {
                    continue;
                }

                auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [object](const auto &ptr)
                                       { return ptr.get() == object; });
                if (it != m_gameObjects.end())
                {
                    (*it)->m_scene = nullptr;
                    m_gameObjects.erase(it);
                }
            }
            m_pendingGameObjectDestruction.clear();
        }
    }
} // namespace Zenith
