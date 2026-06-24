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

    Component *sceneGetComponent(Scene &scene, GameObject &object, std::type_index type)
    {
        return scene.getComponent(object, type);
    }

    const Component *sceneGetComponent(const Scene &scene, const GameObject &object, std::type_index type)
    {
        return scene.getComponent(object, type);
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

    void Scene::onEnter()
    {
        if (m_isEntered)
        {
            return;
        }

        m_isEntered = true;
        flushCommands();
    }

    void Scene::onExit()
    {
        if (!m_isEntered)
        {
            return;
        }

        flushCommands();
        m_isEntered = false;
    }

    void Scene::onUnload()
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
        m_componentRegistry.clear();
        m_isEntered = false;
    }

    void Scene::clear()
    {
        onExit();
        onUnload();
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

    Component *Scene::getComponent(GameObject &object, std::type_index type)
    {
        auto storageIt = m_componentRegistry.find(&object);
        if (storageIt == m_componentRegistry.end())
        {
            return nullptr;
        }

        auto componentIt = storageIt->second.find(type);
        if (componentIt == storageIt->second.end())
        {
            return nullptr;
        }

        return componentIt->second.get();
    }

    const Component *Scene::getComponent(const GameObject &object, std::type_index type) const
    {
        auto storageIt = m_componentRegistry.find(const_cast<GameObject *>(&object));
        if (storageIt == m_componentRegistry.end())
        {
            return nullptr;
        }

        auto componentIt = storageIt->second.find(type);
        if (componentIt == storageIt->second.end())
        {
            return nullptr;
        }

        return componentIt->second.get();
    }

    void Scene::flushCommands()
    {
        for (auto &addition : m_pendingComponentAdditions)
        {
            if (!addition.object || !addition.component)
            {
                continue;
            }

            auto &storage = m_componentRegistry[addition.object];
            if (storage.find(addition.type) != storage.end())
            {
                continue;
            }

            addition.component->setOwner(addition.object);
            if (auto *script = dynamic_cast<Components::ScriptComponent *>(addition.component.get()))
            {
                script->markAttached();
            }

            storage.emplace(addition.type, std::move(addition.component));
        }
        m_pendingComponentAdditions.clear();

        for (const auto &removal : m_pendingComponentRemovals)
        {
            if (!removal.object)
            {
                continue;
            }

            auto storageIt = m_componentRegistry.find(removal.object);
            if (storageIt == m_componentRegistry.end())
            {
                continue;
            }

            auto &storage = storageIt->second;
            auto componentIt = storage.find(removal.type);
            if (componentIt != storage.end())
            {
                if (auto *script = dynamic_cast<Components::ScriptComponent *>(componentIt->second.get()))
                {
                    script->markDetached();
                }
                storage.erase(componentIt);
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
                    m_componentRegistry.erase(object);
                    m_gameObjects.erase(it);
                }
            }
            m_pendingGameObjectDestruction.clear();
        }
    }
} // namespace Zenith
