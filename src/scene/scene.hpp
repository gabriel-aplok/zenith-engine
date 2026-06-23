#pragma once

#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "components/camera.hpp"
#include "scene/game_object.hpp"
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
        void clear();
        void setMeshMetadataProvider(const Render::IMeshMetadataProvider *provider);

        void update(float deltaTime);
        bool buildRenderFrame(RenderFrame &frame, const glm::ivec2 &framebufferSize);
        void render(RenderFrame &frame);
        void render(RenderFrame &frame, const glm::ivec2 &framebufferSize);

        std::size_t gameObjectCount() const { return m_gameObjects.size(); }

    private:
        Components::Camera *findCamera();
        const Components::Camera *findCamera() const;
        std::optional<Render::Bounds> meshBoundsFor(const Components::MeshFilter &filter) const;

        const Render::IMeshMetadataProvider *m_meshMetadataProvider = nullptr;
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    };
} // namespace Zenith
