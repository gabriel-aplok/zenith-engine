#include "systems/script_system.hpp"

#include "components/script_component.hpp"
#include "scene/scene.hpp"

namespace Zenith
{
    void ScriptSystem::onAdd(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            auto *script = object->get_component<Components::ScriptComponent>();
            if (script && script->onAdd)
            {
                script->onAdd();
            }
        }
    }

    void ScriptSystem::onRemove(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onRemove)
            {
                script->onRemove();
            }
        }
    }

    void ScriptSystem::onStart(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && !script->started)
            {
                script->started = true;
                if (script->onStart)
                {
                    script->onStart();
                }
            }
        }
    }

    void ScriptSystem::onStop(Scene &scene)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onDispose)
            {
                script->onDispose();
            }
        }
    }

    void ScriptSystem::preUpdate(Scene &scene, float deltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onPreUpdate)
            {
                script->onPreUpdate(deltaTime);
            }
        }
    }

    void ScriptSystem::update(Scene &scene, float deltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onUpdate)
            {
                script->onUpdate(deltaTime);
            }
        }
    }

    void ScriptSystem::postUpdate(Scene &scene, float deltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onPostUpdate)
            {
                script->onPostUpdate(deltaTime);
            }
        }
    }

    void ScriptSystem::fixedUpdate(Scene &scene, float fixedDeltaTime)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onFixedUpdate)
            {
                script->onFixedUpdate(fixedDeltaTime);
            }
        }
    }

    void ScriptSystem::render(Scene &scene, RenderFrame &frame)
    {
        for (auto &object : scene.gameObjects())
        {
            if (auto *script = object->get_component<Components::ScriptComponent>(); script && script->onRender)
            {
                script->onRender(frame);
            }
        }
    }
} // namespace Zenith
