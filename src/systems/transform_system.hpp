#pragma once

#include "scene/system.hpp"

namespace Zenith
{
    class TransformSystem final : public System
    {
    public:
        void update(Scene &scene, float deltaTime) override;
    };
} // namespace Zenith
