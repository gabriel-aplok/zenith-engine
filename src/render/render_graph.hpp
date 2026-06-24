#pragma once

#include <cstdint>
#include <vector>

#include "render/mesh.hpp"
#include "render/lighting.hpp"
#include "render/render_commands.hpp"

namespace Zenith::Render
{
    enum class RenderPassKind
    {
        Shadow,
        Opaque,
        Transparent,
        Present,
        PostProcess,
        Debug,
        Compute,
    };

    enum class RenderTargetFormat
    {
        Unknown,
        ColorRGBA8,
        ColorRGBA16F,
        Depth24,
        Depth32F,
    };

    struct RenderTargetDesc
    {
        uint16_t width = 0;
        uint16_t height = 0;
        RenderTargetFormat format = RenderTargetFormat::Unknown;
        uint8_t mipLevels = 1;
        bool allowSampling = true;
        bool allowRendering = true;
    };

    struct RenderAttachment
    {
        uint32_t targetId = 0;
        bool clear = false;
        bool preserve = false;
    };

    struct RenderPassDesc
    {
        RenderPassKind kind = RenderPassKind::Opaque;
        uint32_t viewId = 0;
        RenderTargetDesc colorTarget{};
        RenderTargetDesc depthTarget{};
        RenderTargetDesc sourceTarget{};
        ClusteredLightingFrame clusteredLighting{};
        bool useBackbuffer = true;
    };

    struct RenderPass
    {
        RenderPassDesc desc{};
        RenderCommandList commands{};
    };

    class RenderGraph
    {
    public:
        void clear();
        RenderPass& addPass(const RenderPassDesc& desc);
        const std::vector<RenderPass>& passes() const { return m_passes; }
        std::vector<RenderPass>& passes() { return m_passes; }

    private:
        std::vector<RenderPass> m_passes{};
    };

} // namespace Zenith::Render
