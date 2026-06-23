#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace Zenith::Render
{

    struct MeshVertex
    {
        glm::vec3 position{0.0f};
        glm::vec4 color{1.0f};
    };

    struct MeshData
    {
        std::vector<MeshVertex> vertices;
        std::vector<uint16_t> indices;
    };

    struct MeshHandle
    {
        uint32_t id = 0;
    };

    struct MeshDrawCommand
    {
        MeshHandle mesh{};
        glm::mat4 transform{1.0f};
        uint32_t firstIndex = 0;
        uint32_t indexCount = 0;
    };

    struct RenderViewState
    {
        glm::mat4 view{1.0f};
        glm::mat4 projection{1.0f};
    };

} // namespace Zenith::Render
