#pragma once

#include "math/math.hpp"
#include "render/lighting.hpp"
#include "scene/component.hpp"

namespace Zenith::Components
{
    class Light final : public Component
    {
    public:
        Light() = default;
        ~Light() override = default;

        Render::LightType type() const { return m_light.type; }
        void setType(Render::LightType type) { m_light.type = type; }

        const Render::LightData& data() const { return m_light; }
        Render::LightData& data() { return m_light; }

        void setColor(const Vector4& color) { m_light.color = color; }
        void setIntensity(float intensity) { m_light.intensity = intensity; }
        void setRange(float range) { m_light.range = range; }
        void setShadowCasting(bool castsShadow) { m_light.castsShadow = castsShadow; }

    private:
        Render::LightData m_light{};
    };
} // namespace Zenith::Components
