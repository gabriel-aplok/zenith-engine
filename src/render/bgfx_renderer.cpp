#include "render/bgfx_renderer.hpp"

#include <vector>

#include <bgfx/embedded_shader.h>
#include <glm/gtc/type_ptr.hpp>

#include "engine/window_backend.hpp"
#include "log/log.hpp"
#include "render/render_context.hpp"
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

        struct VertexGpu
        {
            float position[3];
            uint32_t color;
        };

        uint32_t packColor(const glm::vec4 &color)
        {
            const uint32_t r = static_cast<uint32_t>(glm::clamp(color.r, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t g = static_cast<uint32_t>(glm::clamp(color.g, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t b = static_cast<uint32_t>(glm::clamp(color.b, 0.0f, 1.0f) * 255.0f + 0.5f);
            const uint32_t a = static_cast<uint32_t>(glm::clamp(color.a, 0.0f, 1.0f) * 255.0f + 0.5f);
            return (a << 24) | (b << 16) | (g << 8) | r;
        }

        bgfx::ShaderHandle loadShader(const uint8_t *bytes, size_t size)
        {
            const bgfx::Memory *memory = bgfx::copy(bytes, static_cast<uint32_t>(size));
            return bgfx::createShader(memory);
        }

        struct ShaderVariant
        {
            const uint8_t *vertexBytes = nullptr;
            size_t vertexSize = 0;
            const uint8_t *fragmentBytes = nullptr;
            size_t fragmentSize = 0;
        };

        ShaderVariant selectShaderVariant(bgfx::RendererType::Enum rendererType)
        {
            switch (rendererType)
            {
            case bgfx::RendererType::OpenGL:
                return {mesh_vs_glsl430, sizeof(mesh_vs_glsl430), mesh_fs_glsl430, sizeof(mesh_fs_glsl430)};
            case bgfx::RendererType::OpenGLES:
                return {mesh_vs_gles300, sizeof(mesh_vs_gles300), mesh_fs_gles300, sizeof(mesh_fs_gles300)};
            case bgfx::RendererType::Vulkan:
                return {mesh_vs_spirv, sizeof(mesh_vs_spirv), mesh_fs_spirv, sizeof(mesh_fs_spirv)};
            case bgfx::RendererType::Direct3D11:
            case bgfx::RendererType::Direct3D12:
            case bgfx::RendererType::Noop:
            case bgfx::RendererType::Agc:
            case bgfx::RendererType::Gnm:
            case bgfx::RendererType::Metal:
            case bgfx::RendererType::Nvn:
            case bgfx::RendererType::Count:
            default:
                return {mesh_vs_dx11, sizeof(mesh_vs_dx11), mesh_fs_dx11, sizeof(mesh_fs_dx11)};
            }
        }

    } // namespace

    bool BgfxRenderer::initialize(RenderContext &context)
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
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

        m_tintUniform = bgfx::createUniform("u_tint", bgfx::UniformType::Vec4);
        if (!bgfx::isValid(m_tintUniform))
        {
            Log::Error("Failed to create bgfx tint uniform");
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

        m_initialized = true;
        return true;
    }

    void BgfxRenderer::shutdown()
    {
        for (auto &[_, mesh] : m_meshes)
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

        m_initialized = false;
    }

    void BgfxRenderer::resize(const glm::ivec2 &framebufferSize)
    {
        m_framebufferSize = framebufferSize;
    }

    Render::MeshHandle BgfxRenderer::uploadMesh(const Render::MeshData &mesh)
    {
        if (!m_initialized || mesh.vertices.empty() || mesh.indices.empty())
        {
            return {};
        }

        std::vector<VertexGpu> vertices;
        vertices.reserve(mesh.vertices.size());
        for (const auto &vertex : mesh.vertices)
        {
            vertices.push_back(VertexGpu{
                .position = {vertex.position.x, vertex.position.y, vertex.position.z},
                .color = packColor(vertex.color),
            });
        }

        const bgfx::Memory *vertexMemory = bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexGpu)));
        const bgfx::Memory *indexMemory = bgfx::copy(mesh.indices.data(), static_cast<uint32_t>(mesh.indices.size() * sizeof(uint16_t)));

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
        m_meshes.emplace(meshId, MeshResource{vertexBuffer, indexBuffer, static_cast<uint32_t>(mesh.indices.size())});
        return Render::MeshHandle{meshId};
    }

    bool BgfxRenderer::updateMesh(Render::MeshHandle meshHandle, const Render::MeshData &mesh)
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
        for (const auto &vertex : mesh.vertices)
        {
            vertices.push_back(VertexGpu{
                .position = {vertex.position.x, vertex.position.y, vertex.position.z},
                .color = packColor(vertex.color),
            });
        }

        const bgfx::Memory *vertexMemory = bgfx::copy(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(VertexGpu)));
        const bgfx::Memory *indexMemory = bgfx::copy(mesh.indices.data(), static_cast<uint32_t>(mesh.indices.size() * sizeof(uint16_t)));

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

    void BgfxRenderer::render(const RenderFrame &frame)
    {
        if (!m_initialized)
        {
            return;
        }

        const RenderViewState &view = frame.view;
        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, packColor(frame.clearColor), 1.0f, 0);
        bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(m_framebufferSize.x), static_cast<uint16_t>(m_framebufferSize.y));
        bgfx::setViewTransform(0, glm::value_ptr(view.view), glm::value_ptr(view.projection));
        bgfx::touch(0);

        const RenderStats &stats = frame.commands.stats();
        bgfx::dbgTextPrintf(0, 1, 0x0f, "frame: commands %u  batches %u  draws %u  indices %u", stats.commandCount, stats.batchCount, stats.drawCount, stats.indexCount);

        const uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW;
        for (const RenderBatch &batch : frame.commands.batches())
        {
            const auto meshIt = m_meshes.find(batch.mesh.id);
            if (meshIt == m_meshes.end())
            {
                continue;
            }

            const MeshResource &mesh = meshIt->second;
            const uint32_t firstIndex = batch.firstIndex;
            const uint32_t indexCount = batch.indexCount == 0 ? mesh.indexCount - firstIndex : batch.indexCount;
            if (indexCount == 0 || firstIndex >= mesh.indexCount)
            {
                continue;
            }

            const float tint[4] = {batch.material.tint.r, batch.material.tint.g, batch.material.tint.b, batch.material.tint.a};
            bgfx::setTransform(glm::value_ptr(batch.transform));
            bgfx::setVertexBuffer(0, mesh.vertexBuffer);
            bgfx::setIndexBuffer(mesh.indexBuffer, firstIndex, indexCount);
            bgfx::setUniform(m_tintUniform, tint);
            bgfx::setState(state);
            bgfx::submit(0, m_program);
        }
    }

} // namespace Zenith::Render
