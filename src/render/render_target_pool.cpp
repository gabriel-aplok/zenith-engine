#include "render/render_target_pool.hpp"

namespace Zenith::Render
{
    namespace
    {
        bgfx::TextureFormat::Enum toBgfxFormat(RenderTargetFormat format)
        {
            switch (format)
            {
            case RenderTargetFormat::ColorRGBA16F:
                return bgfx::TextureFormat::RGBA16F;
            case RenderTargetFormat::Depth24:
                return bgfx::TextureFormat::D24S8;
            case RenderTargetFormat::Depth32F:
                return bgfx::TextureFormat::D32F;
            case RenderTargetFormat::ColorRGBA8:
            case RenderTargetFormat::Unknown:
            default:
                return bgfx::TextureFormat::BGRA8;
            }
        }

        bool isDepthFormat(RenderTargetFormat format)
        {
            return format == RenderTargetFormat::Depth24 || format == RenderTargetFormat::Depth32F;
        }
    }

    RenderTargetHandle RenderTargetPool::acquire(const RenderTargetDesc& desc)
    {
        for (auto& [id, entry] : m_entries)
        {
            if (entry.desc.width == desc.width &&
                entry.desc.height == desc.height &&
                entry.desc.format == desc.format &&
                entry.desc.mipLevels == desc.mipLevels &&
                entry.desc.allowSampling == desc.allowSampling &&
                entry.desc.allowRendering == desc.allowRendering)
            {
                ++entry.refCount;
                return RenderTargetHandle{ id };
            }
        }

        const uint32_t id = m_nextId++;
        bgfx::TextureHandle texture = BGFX_INVALID_HANDLE;
        bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
        if (desc.width > 0 && desc.height > 0)
        {
            const uint64_t flags = desc.allowRendering ? BGFX_TEXTURE_RT : 0;
            if (isDepthFormat(desc.format))
            {
                framebuffer = bgfx::createFrameBuffer(desc.width, desc.height, toBgfxFormat(desc.format), flags);
            }
            else
            {
                framebuffer = bgfx::createFrameBuffer(desc.width, desc.height, toBgfxFormat(desc.format), flags);
            }

            if (bgfx::isValid(framebuffer))
            {
                texture = bgfx::getTexture(framebuffer);
            }
        }

        m_entries.emplace(id, Entry{ .desc = desc, .texture = texture, .framebuffer = framebuffer, .refCount = 1 });
        return RenderTargetHandle{ id };
    }

    void RenderTargetPool::release(RenderTargetHandle handle)
    {
        auto it = m_entries.find(handle.id);
        if (it == m_entries.end())
        {
            return;
        }

        if (it->second.refCount > 0)
        {
            --it->second.refCount;
        }
    }

    void RenderTargetPool::clear()
    {
        for (auto& [_, entry] : m_entries)
        {
            if (bgfx::isValid(entry.framebuffer))
            {
                bgfx::destroy(entry.framebuffer);
            }
        }
        m_entries.clear();
    }

    bgfx::FrameBufferHandle RenderTargetPool::framebuffer(RenderTargetHandle handle) const
    {
        const auto it = m_entries.find(handle.id);
        if (it == m_entries.end())
        {
            return BGFX_INVALID_HANDLE;
        }
        return it->second.framebuffer;
    }

    bgfx::TextureHandle RenderTargetPool::texture(RenderTargetHandle handle) const
    {
        const auto it = m_entries.find(handle.id);
        if (it == m_entries.end())
        {
            return BGFX_INVALID_HANDLE;
        }
        return it->second.texture;
    }

    const RenderTargetDesc* RenderTargetPool::describe(RenderTargetHandle handle) const
    {
        const auto it = m_entries.find(handle.id);
        if (it == m_entries.end())
        {
            return nullptr;
        }
        return &it->second.desc;
    }
} // namespace Zenith::Render
