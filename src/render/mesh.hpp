#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace Zenith::Render
{

    struct MeshVertex
    {
        glm::vec3 position{0.0f};
        glm::vec2 uv{0.0f};
        glm::vec4 color{1.0f};
    };

    struct Bounds
    {
        glm::vec3 center{0.0f};
        glm::vec3 extents{0.5f};

        float radius() const
        {
            return glm::length(extents);
        }
    };

    struct MeshData
    {
        std::vector<MeshVertex> vertices;
        std::vector<uint16_t> indices;
        Bounds bounds{};
    };

    struct MaterialState
    {
        glm::vec4 tint{1.0f};
        uint32_t textureId = 0;
    };

    struct MeshHandle
    {
        uint32_t id = 0;
    };

    struct TextureHandle
    {
        uint32_t id = 0;
    };

    enum class RenderCommandType
    {
        SetTransform,
        SetMaterial,
        BindMesh,
        DrawIndexed,
    };

    struct RenderCommand
    {
        RenderCommandType type = RenderCommandType::DrawIndexed;
        MeshHandle mesh{};
        TextureHandle texture{};
        glm::mat4 transform{1.0f};
        MaterialState material{};
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
    };

    struct RenderBatch
    {
        MeshHandle mesh{};
        TextureHandle texture{};
        glm::mat4 transform{1.0f};
        MaterialState material{};
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
    };

    struct RenderStats
    {
        uint32_t commandCount = 0;
        uint32_t batchCount = 0;
        uint32_t drawCount = 0;
        uint32_t indexCount = 0;
    };

    struct RenderClearState
    {
        uint8_t flags = 0;
        glm::vec4 color{0.08f, 0.09f, 0.11f, 1.0f};
        float depth = 1.0f;
        uint8_t stencil = 0;
    };

    struct RenderViewState
    {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
    };

} // namespace Zenith::Render
