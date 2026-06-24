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
                return bgfx::TextureFormat::RGBA8;
            }
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
        if (desc.width > 0 && desc.height > 0)
        {
            texture = bgfx::createTexture2D(desc.width, desc.height, false, desc.mipLevels, toBgfxFormat(desc.format), desc.allowRendering ? BGFX_TEXTURE_RT : 0);
        }

        bgfx::FrameBufferHandle framebuffer = BGFX_INVALID_HANDLE;
        if (bgfx::isValid(texture))
        {
            const bgfx::TextureHandle attachments[] = { texture };
            framebuffer = bgfx::createFrameBuffer(1, attachments, false);
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
            if (bgfx::isValid(entry.texture))
            {
                bgfx::destroy(entry.texture);
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
