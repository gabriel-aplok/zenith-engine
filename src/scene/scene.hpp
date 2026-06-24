#pragma once

#include <memory>
#include <string>
#include <optional>
#include <typeindex>
#include <vector>

#include "components/camera.hpp"
#include "scene/game_object.hpp"
#include "scene/system.hpp"
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
        void clear();
        void setMeshMetadataProvider(const Render::IMeshMetadataProvider *provider);
        void addSystem(std::unique_ptr<System> system);
        const std::vector<std::unique_ptr<GameObject>> &gameObjects() const { return m_gameObjects; }
        std::vector<std::unique_ptr<GameObject>> &gameObjects() { return m_gameObjects; }
        const glm::ivec2 &framebufferSize() const { return m_framebufferSize; }

        void update(float deltaTime);
        void setFramebufferSize(const glm::ivec2 &framebufferSize);
        void render(RenderFrame &frame);
        void flushCommands();

        void queueComponentAddition(GameObject &object, std::type_index type, std::unique_ptr<Component> component);
        void queueComponentRemoval(GameObject &object, std::type_index type);

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
        std::optional<Render::Bounds> meshBoundsFor(const Components::MeshFilter &filter) const;

        const Render::IMeshMetadataProvider *m_meshMetadataProvider = nullptr;
        glm::ivec2 m_framebufferSize{0, 0};
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
        std::vector<std::unique_ptr<System>> m_systems;
        std::vector<PendingComponentAddition> m_pendingComponentAdditions;
        struct PendingComponentRemoval
        {
            GameObject *object;
            std::type_index type;
        };
        std::vector<GameObject *> m_pendingGameObjectDestruction;
        std::vector<PendingComponentRemoval> m_pendingComponentRemovals;
        friend class RenderSystem;
        friend class CameraSystem;
    };
} // namespace Zenith
