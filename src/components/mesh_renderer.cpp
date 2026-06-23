#include "components/mesh_renderer.hpp"

#include "components/mesh_filter.hpp"
#include "scene/game_object.hpp"

namespace Zenith::Components
{
    void MeshRenderer::render(RenderFrame &frame)
    {
        auto *filter = gameObject().get_component<MeshFilter>();
        if (!filter)
        {
            return;
        }

        const Render::MeshHandle mesh = filter->mesh();
        if (mesh.id == 0)
        {
            return;
        }

        frame.submitMesh(mesh, gameObject().transform().worldMatrix(), m_material);
    }
} // namespace Zenith::Components
