#pragma once

#include <cstdint>
#include <unordered_map>

#include <bgfx/bgfx.h>

#include "render/render_graph.hpp"

namespace Zenith::Render
{
    struct RenderTargetHandle
    {
        uint32_t id = 0;
    };

    class RenderTargetPool
    {
    public:
        RenderTargetHandle acquire(const RenderTargetDesc& desc);
        void release(RenderTargetHandle handle);
        void clear();

        bgfx::FrameBufferHandle framebuffer(RenderTargetHandle handle) const;
        bgfx::TextureHandle texture(RenderTargetHandle handle) const;
        const RenderTargetDesc* describe(RenderTargetHandle handle) const;

    private:
        struct Entry
        {
            RenderTargetDesc desc{};
            bgfx::TextureHandle texture{ BGFX_INVALID_HANDLE };
            bgfx::FrameBufferHandle framebuffer{ BGFX_INVALID_HANDLE };
            uint32_t refCount = 0;
        };

        uint32_t m_nextId = 1;
        std::unordered_map<uint32_t, Entry> m_entries;
    };
} // namespace Zenith::Render
