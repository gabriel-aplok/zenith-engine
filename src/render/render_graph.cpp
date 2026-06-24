#include "render/render_graph.hpp"

namespace Zenith::Render
{
    void RenderGraph::clear()
    {
        m_passes.clear();
    }

    RenderPass& RenderGraph::addPass(const RenderPassDesc& desc)
    {
        m_passes.push_back(RenderPass{ .desc = desc });
        return m_passes.back();
    }

} // namespace Zenith::Render
