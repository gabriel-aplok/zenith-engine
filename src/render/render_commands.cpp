#include "render/render_commands.hpp"

namespace Zenith::Render
{

    void RenderCommandList::clear()
    {
        m_commands.clear();
    }

    void RenderCommandList::reserve(std::size_t drawCount)
    {
        m_commands.reserve(drawCount);
    }

    void RenderCommandList::setView(const RenderViewState &view)
    {
        m_view = view;
    }

    void RenderCommandList::setTransform(const glm::mat4 &transform)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::SetTransform, .transform = transform});
    }

    void RenderCommandList::setMaterial(const MaterialState &material)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::SetMaterial, .material = material});
    }

    void RenderCommandList::bindMesh(MeshHandle mesh)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::BindMesh, .mesh = mesh});
    }

    void RenderCommandList::drawIndexed(uint32_t firstIndex, uint32_t indexCount)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::DrawIndexed, .firstIndex = firstIndex, .indexCount = indexCount});
    }

    void RenderCommandList::drawIndexed(MeshHandle mesh, uint32_t firstIndex, uint32_t indexCount, const glm::mat4 &transform, const MaterialState &material)
    {
        bindMesh(mesh);
        setTransform(transform);
        setMaterial(material);
        drawIndexed(firstIndex, indexCount);
    }

    const RenderViewState &RenderCommandList::view() const
    {
        return m_view;
    }

    const std::vector<RenderCommand> &RenderCommandList::commands() const
    {
        return m_commands;
    }

} // namespace Zenith::Render
