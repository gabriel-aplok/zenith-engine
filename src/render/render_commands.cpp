#include "render/render_commands.hpp"

namespace Zenith::Render {

void RenderCommandList::clear() {
    m_meshDraws.clear();
}

void RenderCommandList::setView(const RenderViewState &view) {
    m_view = view;
}

void RenderCommandList::drawIndexed(MeshHandle mesh, uint32_t firstIndex, uint32_t indexCount, const glm::mat4 &transform) {
    m_meshDraws.push_back(MeshDrawCommand{.mesh = mesh, .transform = transform, .firstIndex = firstIndex, .indexCount = indexCount});
}

const RenderViewState &RenderCommandList::view() const {
    return m_view;
}

const std::vector<MeshDrawCommand> &RenderCommandList::meshDraws() const {
    return m_meshDraws;
}

} // namespace Zenith::Render
