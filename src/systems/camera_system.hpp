#pragma once

#include "scene/system.hpp"

namespace Zenith
{
    class CameraSystem final : public System
    {
    public:
        void render(Scene &scene, RenderFrame &frame) override;
    };
} // namespace Zenith
