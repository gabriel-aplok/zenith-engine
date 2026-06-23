#pragma once

#include "render/mesh.hpp"
#include "scene/component.hpp"

namespace Zenith::Components
{
    class MeshFilter final : public Component
    {
    public:
        MeshFilter() = default;
        ~MeshFilter() override = default;

        Render::MeshHandle mesh() const { return m_mesh; }
        void setMesh(Render::MeshHandle mesh) { m_mesh = mesh; }

        const Render::Bounds &bounds() const { return m_bounds; }
        void setBounds(const Render::Bounds &bounds) { m_bounds = bounds; }

    private:
        Render::MeshHandle m_mesh{};
        Render::Bounds m_bounds{};
    };
} // namespace Zenith::Components
