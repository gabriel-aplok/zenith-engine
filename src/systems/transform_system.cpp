#include "systems/transform_system.hpp"

#include "scene/scene.hpp"

namespace Zenith
{
    void TransformSystem::update(Scene &scene, float deltaTime)
    {
        (void)deltaTime;
        for (auto &object : scene.gameObjects())
        {
            (void)object->transform().localToWorld();
        }
    }
} // namespace Zenith
