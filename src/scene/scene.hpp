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

        void setFramebufferSize(const glm::ivec2 &framebufferSize);
        void flushCommands();

        void queueComponentAddition(GameObject &object, std::type_index type, std::unique_ptr<Component> component);
        void queueComponentRemoval(GameObject &object, std::type_index type);
        Component *getComponent(GameObject &object, std::type_index type);
        const Component *getComponent(const GameObject &object, std::type_index type) const;

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
        struct PendingComponentAddition
        {
            GameObject *object;
            std::type_index type;
            std::unique_ptr<Component> component;
        };
        Components::Camera *findCamera();
        const Components::Camera *findCamera() const;
        glm::ivec2 m_framebufferSize{0, 0};
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
        std::vector<PendingComponentAddition> m_pendingComponentAdditions;
        std::unordered_map<GameObject *, std::unordered_map<std::type_index, std::unique_ptr<Component>>> m_componentRegistry;
        struct PendingComponentRemoval
        {
            GameObject *object;
            std::type_index type;
        };
        std::vector<GameObject *> m_pendingGameObjectDestruction;
        std::vector<PendingComponentRemoval> m_pendingComponentRemovals;
        bool m_isEntered = false;
        friend class RenderSystem;
        friend class CameraSystem;
    };
} // namespace Zenith
