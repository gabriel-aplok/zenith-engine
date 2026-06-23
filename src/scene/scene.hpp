#pragma once

#include <memory>
#include <string>
#include <vector>

#include "components/camera.hpp"
#include "scene/game_object.hpp"

namespace Zenith
{
    class Scene
    {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(const Scene &) = delete;
        Scene &operator=(const Scene &) = delete;

        GameObject &createGameObject(std::string name = {});
        void clear();

        void update(float deltaTime);
        bool buildRenderFrame(RenderFrame &frame, const glm::ivec2 &framebufferSize);
        void render(RenderFrame &frame);
        void render(RenderFrame &frame, const glm::ivec2 &framebufferSize);

        std::size_t gameObjectCount() const { return m_gameObjects.size(); }

    private:
        Components::Camera *findCamera();
        const Components::Camera *findCamera() const;

        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    };
} // namespace Zenith
