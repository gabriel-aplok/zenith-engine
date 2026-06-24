#include "systems/script_system.hpp"

#include "components/script_component.hpp"
#include "components/script_behaviour.hpp"
#include "scene/scene.hpp"

namespace Zenith
{
    void ScriptSystem::onStart(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour() && !script->started)
            {
                script->started = true;
                script->behaviour()->onStart(*object);
            }
        }
    }

    void ScriptSystem::onStop(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onStop(*object);
            }
        }
    }

    void ScriptSystem::preUpdate(Scene &scene, float deltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onPreUpdate(*object, deltaTime);
            }
        }
    }

    void ScriptSystem::update(Scene &scene, float deltaTime)
    {
        onStart(scene);
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onUpdate(*object, deltaTime);
            }
        }
    }

    void ScriptSystem::postUpdate(Scene &scene, float deltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onPostUpdate(*object, deltaTime);
            }
        }
    }

    void ScriptSystem::fixedUpdate(Scene &scene, float fixedDeltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onFixedUpdate(*object, fixedDeltaTime);
            }
        }
    }

    void ScriptSystem::render(Scene &scene, RenderFrame &frame)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->behaviour())
            {
                script->behaviour()->onRender(*object, frame);
            }
        }
    }
} // namespace Zenith
