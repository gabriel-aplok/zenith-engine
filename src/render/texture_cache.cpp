#include "render/texture_cache.hpp"

#include <utility>

#include "resource/resource.hpp"

namespace Zenith::Render
{
    void TextureCache::setUploader(ITextureUploader* uploader)
    {
        m_uploader = uploader;
    }

    TextureHandle TextureCache::acquire(const std::string& key, const ImageSourceData& image)
    {
        auto it = m_entries.find(key);
        if (it != m_entries.end())
        {
            ++it->second.refCount;
            return it->second.handle;
        }

        if (!m_uploader || image.width == 0 || image.height == 0 || image.pixels.empty())
        {
            return {};
        }

        TextureHandle handle = m_uploader->uploadTexture(image.width, image.height, image.pixels.data());
        if (handle.id == 0)
        {
            return {};
        }

        m_entries.emplace(key, Entry{ handle, 1 });
        return handle;
    }

    TextureRef TextureCache::acquireRef(const std::string& key, const ImageSourceData& image)
    {
        return TextureRef{ this, key, acquire(key, image) };
    }

    void TextureCache::release(const std::string& key)
    {
        auto it = m_entries.find(key);
        if (it == m_entries.end())
        {
            return;
        }

        if (it->second.refCount > 0)
        {
            --it->second.refCount;
        }

        if (it->second.refCount == 0)
        {
            if (m_uploader)
            {
                m_uploader->destroyTexture(it->second.handle);
            }
            m_entries.erase(it);
        }
    }

    void TextureCache::clear()
    {
        for (auto& [_, entry] : m_entries)
        {
            if (m_uploader && entry.handle.id != 0)
            {
                m_uploader->destroyTexture(entry.handle);
            }
        }
        m_entries.clear();
    }

    bool TextureCache::has(const std::string& key) const
    {
        return m_entries.find(key) != m_entries.end();
    }

    TextureRef::TextureRef(TextureCache* cache, std::string key, TextureHandle handle)
        : m_cache(cache), m_key(std::move(key)), m_handle(handle)
    {
    }

    TextureRef::~TextureRef()
    {
        reset();
    }

    TextureRef::TextureRef(TextureRef&& other) noexcept
        : m_cache(other.m_cache), m_key(std::move(other.m_key)), m_handle(other.m_handle)
    {
        other.m_cache = nullptr;
        other.m_handle = {};
    }

    TextureRef& TextureRef::operator=(TextureRef&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            m_cache = other.m_cache;
            m_key = std::move(other.m_key);
            m_handle = other.m_handle;
            other.m_cache = nullptr;
            other.m_handle = {};
        }
        return *this;
    }

    void TextureRef::reset()
    {
        if (m_cache && !m_key.empty())
        {
            m_cache->release(m_key);
        }
        m_cache = nullptr;
        m_key.clear();
        m_handle = {};
    }
} // namespace Zenith::Render
