#pragma once

#include <cstdint>
#include <unordered_map>

#include <bgfx/bgfx.h>
#include <glm/glm.hpp>

#include "render/irenderer.hpp"

namespace Zenith::Render
{

    class BgfxRenderer final : public IRenderer
    {
    public:
        bool initialize(RenderContext &context) override;
        void shutdown() override;
        void resize(const glm::ivec2 &framebufferSize) override;
        void render(const RenderFrame &frame) override;

        Render::MeshHandle uploadMesh(const Render::MeshData &mesh) override;
        bool updateMesh(Render::MeshHandle meshHandle, const Render::MeshData &mesh) override;
        void destroyMesh(Render::MeshHandle mesh) override;
        Render::TextureHandle uploadTexture(std::uint32_t width, std::uint32_t height, const std::uint8_t *rgbaPixels) override;
        void destroyTexture(Render::TextureHandle texture) override;

    private:
        struct MeshResource
        {
            bgfx::VertexBufferHandle vertexBuffer{BGFX_INVALID_HANDLE};
            bgfx::IndexBufferHandle indexBuffer{BGFX_INVALID_HANDLE};
            uint32_t indexCount = 0;
        };

        struct TextureResource
        {
            bgfx::TextureHandle texture{BGFX_INVALID_HANDLE};
        };

        bool m_initialized = false;
        glm::ivec2 m_framebufferSize{0, 0};
        uint32_t m_nextMeshId = 1;
        std::unordered_map<uint32_t, MeshResource> m_meshes;
        std::unordered_map<uint32_t, TextureResource> m_textures;
        bgfx::VertexLayout m_vertexLayout;
        bgfx::ProgramHandle m_program{BGFX_INVALID_HANDLE};
        bgfx::UniformHandle m_tintUniform{BGFX_INVALID_HANDLE};
        bgfx::UniformHandle m_textureUniform{BGFX_INVALID_HANDLE};
    };

} // namespace Zenith::Render
