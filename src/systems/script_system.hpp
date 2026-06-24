#pragma once

#include "scene/system.hpp"

namespace Zenith
{
    class ScriptSystem final : public System
    {
    public:
        void onAdd(Scene &scene) override;
        void onRemove(Scene &scene) override;
        void onStart(Scene &scene) override;
        void onStop(Scene &scene) override;
        void preUpdate(Scene &scene, float deltaTime) override;
        void update(Scene &scene, float deltaTime) override;
        void postUpdate(Scene &scene, float deltaTime) override;
        void fixedUpdate(Scene &scene, float fixedDeltaTime) override;
        void render(Scene &scene, RenderFrame &frame) override;
    };
} // namespace Zenith
