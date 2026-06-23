#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "render/mesh.hpp"

namespace Zenith::Render {

class RenderCommandList {
public:
    void clear();
    void reserve(std::size_t drawCount);
    void setView(const RenderViewState &view);
    void drawIndexed(MeshHandle mesh, uint32_t firstIndex = 0, uint32_t indexCount = 0, const glm::mat4 &transform = glm::mat4{1.0f});
    void drawMesh(MeshHandle mesh, const glm::mat4 &transform = glm::mat4{1.0f}) { drawIndexed(mesh, 0, 0, transform); }

    const RenderViewState &view() const;
    const std::vector<MeshDrawCommand> &meshDraws() const;

private:
    RenderViewState m_view{};
    std::vector<MeshDrawCommand> m_meshDraws{};
};

} // namespace Zenith::Render
