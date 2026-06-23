#pragma once

#include <memory>
#include <string>
#include <vector>

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
        void render(RenderFrame &frame);

        std::size_t gameObjectCount() const { return m_gameObjects.size(); }

    private:
        std::vector<std::unique_ptr<GameObject>> m_gameObjects;
    };
} // namespace Zenith
