#pragma once

#include <memory>
#include <vector>

#include "scene/system.hpp"

namespace Zenith
{
    class Scene;

    class SystemRegistry
    {
    public:
        void addSystem(std::unique_ptr<System> system);
        void bindScene(Scene *scene);
        void update(Scene &scene, float deltaTime);
        void render(Scene &scene, RenderFrame &frame);
        void clear();

    private:
        std::vector<std::unique_ptr<System>> m_systems;
        Scene *m_activeScene = nullptr;
    };
} // namespace Zenith
