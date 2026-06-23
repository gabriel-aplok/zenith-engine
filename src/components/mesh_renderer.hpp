#pragma once

#include "render/mesh.hpp"
#include "scene/component.hpp"

namespace Zenith::Components
{
    class MeshRenderer final : public Component
    {
    public:
        MeshRenderer() = default;
        ~MeshRenderer() override = default;

        void render(RenderFrame &frame) override;

        void setMaterial(const Render::MaterialState &material) { m_material = material; }
        const Render::MaterialState &material() const { return m_material; }

    private:
        Render::MaterialState m_material{};
    };
} // namespace Zenith::Components
