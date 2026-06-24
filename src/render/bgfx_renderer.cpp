#include "render/bgfx_renderer.hpp"

#include <vector>

#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

#include "math/math.hpp"
#include "engine/window_backend.hpp"
#include "log/log.hpp"
#include "render/render_context.hpp"
#include "render/mesh_builder.hpp"
#include "render/shaders/mesh_fs_dx11.bin.h"
#include "render/shaders/mesh_fs_gles300.bin.h"
#include "render/shaders/mesh_fs_glsl430.bin.h"
#include "render/shaders/mesh_fs_spirv.bin.h"
#include "render/shaders/mesh_vs_dx11.bin.h"
#include "render/shaders/mesh_vs_gles300.bin.h"
#include "render/shaders/mesh_vs_glsl430.bin.h"
#include "render/shaders/mesh_vs_spirv.bin.h"

namespace Zenith::Render
{

    namespace
    {
        constexpr uint8_t ClearColorFlag = 0x01;
        constexpr uint8_t ClearDepthFlag = 0x02;
        constexpr uint8_t ClearStencilFlag = 0x04;

        struct VertexGpu
        {
            float position[3];
            float uv[2];
            uint32_t color;
        };

        uint32_t packColor(const Vector4& color)
        {
            const uint32_t r = static_cast<uint32_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t g = static_cast<uint32_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t b = static_cast<uint32_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t a = static_cast<uint32_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f + 0.5f);
            return (a << 24) | (b << 16) | (g << 8) | r;
        }

        VertexGpu toGpuVertex(const MeshVertex& vertex)
        {
            return { {vertex.position.x, vertex.position.y, vertex.position.z}, {vertex.uv.x, vertex.uv.y}, packColor(vertex.color) };
        }

        bgfx::ShaderHandle loadShader(const uint8_t* bytes, size_t size)
        {
            const bgfx::Memory* memory = bgfx::copy(bytes, static_cast<uint32_t>(size));
            return bgfx::createShader(memory);
        }

        struct ShaderVariant
        {
            const uint8_t* vertexBytes = nullptr;
            size_t vertexSize = 0;
            const uint8_t* fragmentBytes = nullptr;
            size_t fragmentSize = 0;
        };

        ShaderVariant selectShaderVariant(bgfx::RendererType::Enum rendererType)
        {
            switch (rendererType)
            {
            case bgfx::RendererType::OpenGL:
                return { mesh_vs_glsl430, sizeof(mesh_vs_glsl430), mesh_fs_glsl430, sizeof(mesh_fs_glsl430) };
            case bgfx::RendererType::OpenGLES:
                return { mesh_vs_gles300, sizeof(mesh_vs_gles300), mesh_fs_gles300, sizeof(mesh_fs_gles300) };
            case bgfx::RendererType::Vulkan:
                return { mesh_vs_spirv, sizeof(mesh_vs_spirv), mesh_fs_spirv, sizeof(mesh_fs_spirv) };
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12:
            case bgfx::RendererType::Noop:
            case bgfx::RendererType::Agc:
            case bgfx::RendererType::Gnm:
            case bgfx::RendererType::Metal:
            case bgfx::RendererType::Nvn:
            case bgfx::RendererType::Count:
            default:
                return { mesh_vs_dx11, sizeof(mesh_vs_dx11), mesh_fs_dx11, sizeof(mesh_fs_dx11) };
            }
        }

    } // namespace

    bool BgfxRenderer::initialize(RenderContext& context)
    {
        if (m_initialized)
        {
            return true;
        }

        if (context.graphicsApi() != GraphicsApi::Bgfx)
        {
            Log::Error("BgfxRenderer requires the bgfx backend");
            return false;
        }

        m_framebufferSize = context.framebufferSize();

        m_vertexLayout.begin()
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        m_tintUniform = bgfx::createUniform("u_tint", bgfx::UniformType::Vec4);
        if (!bgfx::isValid(m_tintUniform))
        {
            Log::Error("Failed to create bgfx tint uniform");
            return false;
        }

        m_textureUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
        if (!bgfx::isValid(m_textureUniform))
        {
            Log::Error("Failed to create bgfx texture uniform");
            return false;
        }

        const ShaderVariant shaders = selectShaderVariant(bgfx::getRendererType());
        const bgfx::ShaderHandle vertexShader = loadShader(shaders.vertexBytes, shaders.vertexSize);
        const bgfx::ShaderHandle fragmentShader = loadShader(shaders.fragmentBytes, shaders.fragmentSize);
        m_program = bgfx::createProgram(vertexShader, fragmentShader, true);
        if (!bgfx::isValid(m_program))
        {
            Log::Error("Failed to create bgfx mesh program");
            return false;
        }

        m_fullscreenQuad = uploadMesh(MeshBuilder::makeFullscreenQuad());

        m_initialized = true;
        return true;
    }

    void BgfxRenderer::shutdown()
    {
        m_targetPool.clear();
        if (m_fullscreenQuad.id != 0)
        {
            destroyMesh(m_fullscreenQuad);
            m_fullscreenQuad = {};
        }
        for (auto& [_, mesh] : m_meshes)
        {
            if (bgfx::isValid(mesh.vertexBuffer))
            {
                bgfx::destroy(mesh.vertexBuffer);
            }
            if (bgfx::isValid(mesh.indexBuffer))
            {
                bgfx::destroy(mesh.indexBuffer);
            }
        }
        m_meshes.clear();

        if (bgfx::isValid(m_program))
        {
            bgfx::destroy(m_program);
            m_program = BGFX_INVALID_HANDLE;
        }

        if (bgfx::isValid(m_tintUniform))
        {
            bgfx::destroy(m_tintUniform);
            m_tintUniform = BGFX_INVALID_HANDLE;
        }

        if (bgfx::isValid(m_textureUniform))
        {
            bgfx::destroy(m_textureUniform);
            m_textureUniform = BGFX_INVALID_HANDLE;
        }

        for (auto& [_, texture] : m_textures)
        {
            if (bgfx::isValid(texture.texture))
            {
                bgfx::destroy(texture.texture);
            }
        }
        m_textures.clear();

        m_initialized = false;
    }

    void BgfxRenderer::resize(const IVector2& framebufferSize)
    {
        m_framebufferSize = framebufferSize;
    }

    void BgfxRenderer::prepareClusteredLighting(const Render::ClusteredLightingFrame& lighting)
    {
        bgfx::dbgTextPrintf(0, 2, 0x0f, "cluster grid %ux%ux%u lights %u",
            lighting.grid.config.clusterX,
            lighting.grid.config.clusterY,
            lighting.grid.config.clusterZ,
            static_cast<uint32_t>(lighting.lights.size()));
    }

    Render::MeshHandle BgfxRenderer::uploadMesh(const Render::MeshData& mesh)
    {
        if (!m_initialized || mesh.vertices.empty() || mesh.indices.empty())
        {
            return {};
        }

        std::vector<VertexGpu> vertices;
        vertices.reserve(mesh.vertices.size());
        for (const auto& vertex : mesh.vertices)
        {
            vertices.push_back(toGpuVertex(vertex));
        }

        const bgfx::Memory* vertexMemory = bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexGpu)));
        const bgfx::Memory* indexMemory = bgfx::copy(mesh.indices.data(), static_cast<uint32_t>(mesh.indices.size() * sizeof(uint16_t)));

        const bgfx::VertexBufferHandle vertexBuffer = bgfx::createVertexBuffer(vertexMemory, m_vertexLayout);
        const bgfx::IndexBufferHandle indexBuffer = bgfx::createIndexBuffer(indexMemory);
        if (!bgfx::isValid(vertexBuffer) || !bgfx::isValid(indexBuffer))
        {
            if (bgfx::isValid(vertexBuffer))
            {
                bgfx::destroy(vertexBuffer);
            }
            if (bgfx::isValid(indexBuffer))
            {
                bgfx::destroy(indexBuffer);
            }
            return {};
        }

        const uint32_t meshId = m_nextMeshId++;
        m_meshes.emplace(meshId, MeshResource{ vertexBuffer, indexBuffer, static_cast<uint32_t>(mesh.indices.size()) });
        return Render::MeshHandle{ meshId };
    }

    bool BgfxRenderer::updateMesh(Render::MeshHandle meshHandle, const Render::MeshData& mesh)
    {
        if (!m_initialized || meshHandle.id == 0 || mesh.vertices.empty() || mesh.indices.empty())
        {
            return false;
        }

        const auto it = m_meshes.find(meshHandle.id);
        if (it == m_meshes.end())
        {
            return false;
        }

        std::vector<VertexGpu> vertices;
        vertices.reserve(mesh.vertices.size());
        for (const auto& vertex : mesh.vertices)
        {
            vertices.push_back(toGpuVertex(vertex));
        }

        const bgfx::Memory* vertexMemory = bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexGpu)));
        const bgfx::Memory* indexMemory = bgfx::copy(mesh.indices.data(), static_cast<uint32_t>(mesh.indices.size() * sizeof(uint16_t)));

        const bgfx::VertexBufferHandle vertexBuffer = bgfx::createVertexBuffer(vertexMemory, m_vertexLayout);
        const bgfx::IndexBufferHandle indexBuffer = bgfx::createIndexBuffer(indexMemory);
        if (!bgfx::isValid(vertexBuffer) || !bgfx::isValid(indexBuffer))
        {
            if (bgfx::isValid(vertexBuffer))
            {
                bgfx::destroy(vertexBuffer);
            }
            if (bgfx::isValid(indexBuffer))
            {
                bgfx::destroy(indexBuffer);
            }
            return false;
        }

        if (bgfx::isValid(it->second.vertexBuffer))
        {
            bgfx::destroy(it->second.vertexBuffer);
        }
        if (bgfx::isValid(it->second.indexBuffer))
        {
            bgfx::destroy(it->second.indexBuffer);
        }

        it->second.vertexBuffer = vertexBuffer;
        it->second.indexBuffer = indexBuffer;
        it->second.indexCount = static_cast<uint32_t>(mesh.indices.size());
        return true;
    }

    void BgfxRenderer::destroyMesh(Render::MeshHandle mesh)
    {
        const auto it = m_meshes.find(mesh.id);
        if (it == m_meshes.end())
        {
            return;
        }

        if (bgfx::isValid(it->second.vertexBuffer))
        {
            bgfx::destroy(it->second.vertexBuffer);
        }
        if (bgfx::isValid(it->second.indexBuffer))
        {
            bgfx::destroy(it->second.indexBuffer);
        }
        m_meshes.erase(it);
    }

    void BgfxRenderer::renderGraph(const Render::RenderGraph& graph)
    {
        for (const Render::RenderPass& pass : graph.passes())
        {
            renderPass(pass);
        }
    }

    void BgfxRenderer::renderPass(const Render::RenderPass& pass)
    {
        switch (pass.desc.kind)
        {
        case Render::RenderPassKind::Shadow:
            renderShadowPass(pass);
            break;
        case Render::RenderPassKind::Opaque:
        case Render::RenderPassKind::Transparent:
            renderSceneLightingPass(pass);
            break;
        case Render::RenderPassKind::Present:
            renderPresentPass(pass);
            break;
        case Render::RenderPassKind::PostProcess:
            renderSceneLightingPass(pass);
            break;
        case Render::RenderPassKind::Debug:
        case Render::RenderPassKind::Compute:
        default:
            renderOpaquePass(pass);
            break;
        }
    }

    void BgfxRenderer::renderShadowPass(const Render::RenderPass& pass)
    {
        if (pass.desc.depthTarget.width == 0 || pass.desc.depthTarget.height == 0)
        {
            return;
        }

        auto handle = m_targetPool.acquire(pass.desc.depthTarget);
        const auto fb = m_targetPool.framebuffer(handle);
        if (bgfx::isValid(fb))
        {
            bgfx::setViewFrameBuffer(static_cast<uint16_t>(pass.desc.viewId), fb);
        }
        bgfx::setViewRect(static_cast<uint16_t>(pass.desc.viewId), 0, 0, pass.desc.depthTarget.width, pass.desc.depthTarget.height);
        bgfx::setViewClear(static_cast<uint16_t>(pass.desc.viewId), BGFX_CLEAR_DEPTH, 0, 1.0f, 0);
        bgfx::touch(static_cast<uint16_t>(pass.desc.viewId));
        m_targetPool.release(handle);
    }

    void BgfxRenderer::renderSceneLightingPass(const Render::RenderPass& pass)
    {
        prepareClusteredLighting(pass.desc.clusteredLighting);
        renderOpaquePass(pass);
    }

    void BgfxRenderer::renderOpaquePass(const Render::RenderPass& pass)
    {
        if (pass.desc.useBackbuffer)
        {
            bgfx::setViewFrameBuffer(static_cast<uint16_t>(pass.desc.viewId), BGFX_INVALID_HANDLE);
        }
        else if (pass.desc.colorTarget.width > 0 && pass.desc.colorTarget.height > 0)
        {
            auto handle = m_targetPool.acquire(pass.desc.colorTarget);
            const auto fb = m_targetPool.framebuffer(handle);
            if (bgfx::isValid(fb))
            {
                bgfx::setViewFrameBuffer(static_cast<uint16_t>(pass.desc.viewId), fb);
            }
            m_targetPool.release(handle);
        }

        const uint16_t width = pass.desc.colorTarget.width > 0 ? pass.desc.colorTarget.width : static_cast<uint16_t>(m_framebufferSize.x);
        const uint16_t height = pass.desc.colorTarget.height > 0 ? pass.desc.colorTarget.height : static_cast<uint16_t>(m_framebufferSize.y);
        bgfx::setViewRect(static_cast<uint16_t>(pass.desc.viewId), 0, 0, width, height);
        bgfx::setViewClear(static_cast<uint16_t>(pass.desc.viewId), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, packColor(Vector4{ 0.08f, 0.09f, 0.11f, 1.0f }), 1.0f, 0);
        bgfx::setViewTransform(static_cast<uint16_t>(pass.desc.viewId), glm::value_ptr(pass.commands.view().view), glm::value_ptr(pass.commands.view().projection));
        bgfx::touch(static_cast<uint16_t>(pass.desc.viewId));

        const Render::RenderStats& stats = pass.commands.stats();
        bgfx::dbgTextPrintf(0, 1, 0x0f, "frame: commands %u  batches %u  draws %u  indices %u", stats.commandCount, stats.batchCount, stats.drawCount, stats.indexCount);

        const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
        for (const RenderBatch& batch : pass.commands.batches())
        {
            const auto meshIt = m_meshes.find(batch.mesh.id);
            if (meshIt == m_meshes.end())
            {
                continue;
            }

            const MeshResource& mesh = meshIt->second;
            const uint32_t firstIndex = batch.firstIndex;
            const uint32_t indexCount = batch.indexCount == 0 ? mesh.indexCount - firstIndex : batch.indexCount;
            if (indexCount == 0 || firstIndex >= mesh.indexCount)
            {
                continue;
            }

            const float tint[4] = { batch.material.tint.r, batch.material.tint.g, batch.material.tint.b, batch.material.tint.a };
            bgfx::setTransform(glm::value_ptr(batch.transform));
            bgfx::setVertexBuffer(0, mesh.vertexBuffer);
            bgfx::setIndexBuffer(mesh.indexBuffer, firstIndex, indexCount);
            bgfx::setUniform(m_tintUniform, tint);
            if (batch.texture.id != 0)
            {
                const auto textureIt = m_textures.find(batch.texture.id);
                if (textureIt != m_textures.end() && bgfx::isValid(textureIt->second.texture))
                {
                    bgfx::setTexture(0, m_textureUniform, textureIt->second.texture);
                }
            }
            bgfx::setState(state);
            bgfx::submit(static_cast<uint16_t>(pass.desc.viewId), m_program);
        }
    }

    void BgfxRenderer::renderPresentPass(const Render::RenderPass& pass)
    {
        if (pass.desc.useBackbuffer)
        {
            return;
        }

        const float identity[16] =
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
        };
        bgfx::setViewTransform(static_cast<uint16_t>(pass.desc.viewId), identity, identity);
        bgfx::setViewRect(static_cast<uint16_t>(pass.desc.viewId), 0, 0, static_cast<uint16_t>(m_framebufferSize.x), static_cast<uint16_t>(m_framebufferSize.y));

        auto sourceHandle = m_targetPool.acquire(pass.desc.sourceTarget);
        const auto sourceTexture = m_targetPool.texture(sourceHandle);
        if (bgfx::isValid(sourceTexture) && m_fullscreenQuad.id != 0)
        {
            const auto meshIt = m_meshes.find(m_fullscreenQuad.id);
            if (meshIt != m_meshes.end())
            {
                const float tint[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                bgfx::setTexture(0, m_textureUniform, sourceTexture);
                bgfx::setUniform(m_tintUniform, tint);
                const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_ALWAYS | BGFX_STATE_CULL_CW;
                bgfx::setState(state);
                bgfx::setTransform(identity);
                bgfx::setVertexBuffer(0, meshIt->second.vertexBuffer);
                bgfx::setIndexBuffer(meshIt->second.indexBuffer);
                bgfx::submit(static_cast<uint16_t>(pass.desc.viewId), m_program);
            }
        }
        m_targetPool.release(sourceHandle);
    }

    void BgfxRenderer::render(const RenderFrame& frame)
    {
        if (!m_initialized)
        {
            return;
        }

        if (!frame.graph.passes().empty())
        {
            renderGraph(frame.graph);
            return;
        }

        Render::RenderPass fallbackPass{};
        fallbackPass.desc.kind = Render::RenderPassKind::Opaque;
        fallbackPass.desc.viewId = 0;
        fallbackPass.commands = frame.commands;
        fallbackPass.commands.setView(frame.view);
        renderOpaquePass(fallbackPass);
    }

    Render::TextureHandle BgfxRenderer::uploadTexture(uint32_t width, uint32_t height, const uint8_t* rgbaPixels)
    {
        if (rgbaPixels == nullptr || width == 0 || height == 0)
        {
            return {};
        }

        const bgfx::Memory* memory = bgfx::copy(rgbaPixels, static_cast<uint32_t>(width * height * 4));
        const bgfx::TextureHandle texture = bgfx::createTexture2D(static_cast<uint16_t>(width), static_cast<uint16_t>(height), false, 1, bgfx::TextureFormat::RGBA8, 0, memory);
        if (!bgfx::isValid(texture))
        {
            return {};
        }

        const uint32_t id = texture.idx + 1;
        m_textures.emplace(id, TextureResource{ texture });
        return Render::TextureHandle{ id };
    }

    void BgfxRenderer::destroyTexture(Render::TextureHandle texture)
    {
        if (texture.id == 0)
        {
            return;
        }

        const auto it = m_textures.find(texture.id);
        if (it == m_textures.end())
        {
            return;
        }

        if (bgfx::isValid(it->second.texture))
        {
            bgfx::destroy(it->second.texture);
        }
        m_textures.erase(it);
    }

} // namespace Zenith::Render
