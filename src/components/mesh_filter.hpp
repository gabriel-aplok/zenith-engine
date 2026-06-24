#pragma once

#include <optional>

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
        void setMesh(Render::MeshHandle mesh, std::optional<Render::Bounds> bounds = std::nullopt)
        {
            m_mesh = mesh;
            m_bounds = bounds;
        }

        const std::optional<Render::Bounds> &bounds() const { return m_bounds; }

    private:
        Render::MeshHandle m_mesh{};
        std::optional<Render::Bounds> m_bounds{};
    };
} // namespace Zenith::Components
