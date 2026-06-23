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

    private:
        Render::MeshHandle m_mesh{};
    };
} // namespace Zenith::Components
