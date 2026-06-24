#include "scene/scene.hpp"

#include <algorithm>

#include "engine/input.hpp"
#include "components/script_component.hpp"

namespace Zenith
{
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
        m_pendingCommands.push_back(PendingCommand{PendingCommandType::DestroyGameObject, &object});
    }

    void Scene::onLoad()
    {
        flushCommands();
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
        m_gameObjects.clear();
        m_componentRegistry.clear();
        m_pendingCommands.clear();
        m_isEntered = false;
    }

    void Scene::clear()
    {
        onExit();
        onUnload();
    }

    void Scene::setFramebufferSize(const glm::ivec2 &framebufferSize)
    {
        m_framebufferSize = framebufferSize;
    }

    void Scene::setInputState(const InputState *inputState)
    {
        m_inputState = inputState;
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

    void Scene::queueComponentRemoval(GameObject &object, std::type_index type)
    {
        m_pendingCommands.push_back(PendingCommand{PendingCommandType::RemoveComponent, &object, type});
    }

    void Scene::queueComponentAddition(GameObject &object, std::type_index type, std::unique_ptr<Component> component)
    {
        m_pendingCommands.push_back(PendingCommand{PendingCommandType::AddComponent, &object, type, std::move(component)});
    }

    namespace
    {
        void notifyScriptRemoval(Component *component, GameObject &object)
        {
            auto *script = dynamic_cast<Components::ScriptComponent *>(component);
            if (!script || !script->behaviour())
            {
                return;
            }

            if (script->started)
            {
                script->behaviour()->onStop(object);
            }
            script->behaviour()->onRemove(object);
            script->started = false;
        }
    } // namespace

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
        for (auto &command : m_pendingCommands)
        {
            switch (command.type)
            {
            case PendingCommandType::AddComponent:
                if (command.object && command.component)
                {
                    auto &storage = m_componentRegistry[command.object];
                    if (storage.find(command.componentType) == storage.end())
                    {
                        command.component->setOwner(command.object);
                        storage.emplace(command.componentType, std::move(command.component));
                    }
                }
                break;

            case PendingCommandType::RemoveComponent:
                if (command.object)
                {
                    auto storageIt = m_componentRegistry.find(command.object);
                    if (storageIt != m_componentRegistry.end())
                    {
                        auto &storage = storageIt->second;
                        auto componentIt = storage.find(command.componentType);
                        if (componentIt != storage.end())
                        {
                            notifyScriptRemoval(componentIt->second.get(), *command.object);
                            storage.erase(componentIt);
                        }
                    }
                }
                break;

            case PendingCommandType::DestroyGameObject:
                if (command.object)
                {
                    auto it = std::find_if(m_gameObjects.begin(), m_gameObjects.end(), [object = command.object](const auto &ptr)
                                           { return ptr.get() == object; });
                    if (it != m_gameObjects.end())
                    {
                        (*it)->m_scene = nullptr;
                        m_componentRegistry.erase(command.object);
                        m_gameObjects.erase(it);
                    }
                }
                break;
            }
        }
        m_pendingCommands.clear();
    }
} // namespace Zenith
