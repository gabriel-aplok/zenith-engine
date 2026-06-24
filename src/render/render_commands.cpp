#include "render/render_commands.hpp"

namespace Zenith::Render
{

    void RenderCommandList::clear()
    {
        m_commands.clear();
        m_batches.clear();
        m_stats = {};
        m_dirty = false;
    }

    void RenderCommandList::reserve(std::size_t drawCount)
    {
        m_commands.reserve(drawCount);
    }

    void RenderCommandList::markDirty()
    {
        m_dirty = true;
    }

    void RenderCommandList::setView(const RenderViewState &view)
    {
        m_view = view;
    }

    void RenderCommandList::setTransform(const glm::mat4 &transform)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::SetTransform, .transform = transform});
        markDirty();
    }

    void RenderCommandList::setMaterial(const MaterialState &material)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::SetMaterial, .material = material});
        markDirty();
    }

    void RenderCommandList::bindMesh(MeshHandle mesh)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::BindMesh, .mesh = mesh});
        markDirty();
    }

    void RenderCommandList::drawIndexed(uint32_t firstIndex, uint32_t indexCount)
    {
        m_commands.push_back(RenderCommand{.type = RenderCommandType::DrawIndexed, .firstIndex = firstIndex, .indexCount = indexCount});
        markDirty();
    }

    void RenderCommandList::drawIndexed(MeshHandle mesh, uint32_t firstIndex, uint32_t indexCount, const glm::mat4 &transform, const MaterialState &material)
    {
        bindMesh(mesh);
        setTransform(transform);
        setMaterial(material);
        drawIndexed(firstIndex, indexCount);
    }

    void RenderCommandList::finalize()
    {
        if (!m_dirty)
        {
            return;
        }

        m_batches.clear();
        m_stats = {};
        MeshHandle currentMesh{};
        glm::mat4 currentTransform{1.0f};
        MaterialState currentMaterial{};

        for (const RenderCommand &command : m_commands)
        {
            switch (command.type)
            {
            case RenderCommandType::BindMesh:
                currentMesh = command.mesh;
                break;
            case RenderCommandType::SetTransform:
                currentTransform = command.transform;
                break;
            case RenderCommandType::SetMaterial:
                currentMaterial = command.material;
                break;
            case RenderCommandType::DrawIndexed:
                m_batches.push_back(RenderBatch{
                    .mesh = currentMesh,
                    .texture = {currentMaterial.textureId},
                    .transform = currentTransform,
                    .material = currentMaterial,
                    .firstIndex = command.firstIndex,
                    .indexCount = command.indexCount,
                });
                ++m_stats.batchCount;
                ++m_stats.drawCount;
                m_stats.indexCount += command.indexCount;
                break;
            }
        }

        m_stats.commandCount = static_cast<uint32_t>(m_commands.size());
        m_dirty = false;
    }

    const RenderViewState &RenderCommandList::view() const
    {
        return m_view;
    }

    const std::vector<RenderCommand> &RenderCommandList::commands() const
    {
        return m_commands;
    }

    const std::vector<RenderBatch> &RenderCommandList::batches() const
    {
        return m_batches;
    }

    const RenderStats &RenderCommandList::stats() const
    {
        return m_stats;
    }

} // namespace Zenith::Render
