#pragma once

#include "render/render_submission.hpp"

namespace Zenith
{
    class Scene;

    class System
    {
    public:
        virtual ~System() = default;
        virtual void onAdd(Scene &scene) {}
        virtual void onRemove(Scene &scene) {}
        virtual void onStart(Scene &scene) {}
        virtual void onStop(Scene &scene) {}
        virtual void preUpdate(Scene &scene, float deltaTime) {}
        virtual void update(Scene &scene, float deltaTime) {}
        virtual void postUpdate(Scene &scene, float deltaTime) {}
        virtual void fixedUpdate(Scene &scene, float fixedDeltaTime) {}
        virtual void preRender(Scene &scene, RenderFrame &frame) {}
        virtual void render(Scene &scene, RenderFrame &frame) {}
        virtual void postRender(Scene &scene, RenderFrame &frame) {}
    };
} // namespace Zenith
