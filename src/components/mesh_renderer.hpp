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

        void setMaterial(const Render::MaterialState &material) { m_material = material; }
        const Render::MaterialState &material() const { return m_material; }
        void setTexture(Render::TextureHandle texture) { m_material.textureId = texture.id; }

    private:
        Render::MaterialState m_material{};
    };
} // namespace Zenith::Components
