#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

#include "render/mesh.hpp"
#include "resource/resource.hpp"

namespace Zenith
{
    namespace Render
    {
        class TextureRef;

        class ITextureUploader
        {
        public:
            virtual ~ITextureUploader() = default;

            virtual TextureHandle uploadTexture(std::uint32_t width, std::uint32_t height, const std::uint8_t* rgbaPixels) = 0;
            virtual void destroyTexture(TextureHandle texture) = 0;
        };

        class TextureCache
        {
        public:
            void setUploader(ITextureUploader* uploader);

            TextureHandle acquire(const std::string& key, const ImageSourceData& image);
            TextureRef acquireRef(const std::string& key, const ImageSourceData& image);
            void release(const std::string& key);
            void clear();

            bool has(const std::string& key) const;

        private:
            struct Entry
            {
                TextureHandle handle{};
                uint32_t refCount = 0;
            };

            ITextureUploader* m_uploader = nullptr;
            std::unordered_map<std::string, Entry> m_entries;
        };

        class TextureRef
        {
        public:
            TextureRef() = default;
            TextureRef(TextureCache* cache, std::string key, TextureHandle handle);
            ~TextureRef();

            TextureRef(const TextureRef&) = delete;
            TextureRef& operator=(const TextureRef&) = delete;
            TextureRef(TextureRef&& other) noexcept;
            TextureRef& operator=(TextureRef&& other) noexcept;

            TextureHandle handle() const { return m_handle; }
            explicit operator bool() const { return m_handle.id != 0; }
            void reset();

        private:
            TextureCache* m_cache = nullptr;
            std::string m_key;
            TextureHandle m_handle{};
        };
    } // namespace Render
} // namespace Zenith
