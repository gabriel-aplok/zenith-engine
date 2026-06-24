#pragma once

#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <type_traits>
#include <vector>

#include "components/camera.hpp"
#include "scene/game_object.hpp"
#include "render/render_submission.hpp"
#include "render/mesh_cache.hpp"

namespace Zenith
{
    class InputState;
    namespace Components
    {
        class MeshFilter;
    }

    class Scene
    {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;

        GameObject &createGameObject(std::string name = {});
        void destroyGameObject(GameObject &object);
        void onLoad();
        void onEnter();
        void onExit();
        void onUnload();
        void clear();
        const std::vector<std::unique_ptr<GameObject>> &gameObjects() const { return m_gameObjects; }
        std::vector<std::unique_ptr<GameObject>> &gameObjects() { return m_gameObjects; }
        const glm::ivec2 &framebufferSize() const { return m_framebufferSize; }
        const InputState *input() const { return m_inputState; }

        void setFramebufferSize(const glm::ivec2 &framebufferSize);
        void setInputState(const InputState *inputState);
        void flushCommands();

        void queueComponentAddition(GameObject &object, std::type_index type, std::unique_ptr<Component> component);
        void queueComponentRemoval(GameObject &object, std::type_index type);
        Component *getComponent(GameObject &object, std::type_index type);
        const Component *getComponent(const GameObject &object, std::type_index type) const;

        template <typename T, typename... Args>
        T &addComponent(GameObject &object, Args &&...args)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T &componentRef = *component;
            queueComponentAddition(object, std::type_index(typeid(T)), std::move(component));
            return componentRef;
        }

        template <typename T>
        T *getComponent(GameObject &object)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<T *>(getComponent(object, std::type_index(typeid(T))));
        }

        template <typename T>
        const T *getComponent(const GameObject &object) const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<const T *>(getComponent(object, std::type_index(typeid(T))));
        }

        template <typename T>
        bool removeComponent(GameObject &object)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            queueComponentRemoval(object, std::type_index(typeid(T)));
            return true;
        }

        template <typename T>
        T *tryGetComponent(GameObject &object)
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<T *>(getComponent(object, std::type_index(typeid(T))));
        }

        template <typename T>
        const T *tryGetComponent(const GameObject &object) const
        {
            static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");
            return dynamic_cast<const T *>(getComponent(object, std::type_index(typeid(T))));
        }

        std::size_t gameObjectCount() const { return m_gameObjects.size(); }

    private:
        enum class PendingCommandType
        {
            AddComponent,
            RemoveComponent,
            DestroyGameObject,
        };

        struct PendingCommand
        {
            PendingCommandType type;
            GameObject *object = nullptr;
            std::type_index componentType{typeid(void)};
            std::unique_ptr<Component> component;
        };
        Components::Camera *findCamera();
        const Components::Camera *findCamera() const;
        glm::ivec2 m_framebufferSize{0, 0};
        const InputState *m_inputState = nullptr;
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
        std::unordered_map<GameObject *, std::unordered_map<std::type_index, std::unique_ptr<Component>>> m_componentRegistry;
        std::vector<PendingCommand> m_pendingCommands;
        friend class RenderSystem;
        friend class CameraSystem;
    };
} // namespace Zenith
