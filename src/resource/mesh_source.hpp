#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace Zenith
{
    struct MeshSourceVertex
    {
        glm::vec3 position{0.0f};
        glm::vec2 uv{0.0f};
        glm::vec4 color{1.0f};
    };

    struct MeshSourceBounds
    {
        glm::vec3 center{0.0f};
        glm::vec3 extents{0.5f};
    };

    struct MeshSource
    {
        std::vector<MeshSourceVertex> vertices;
        std::vector<std::uint16_t> indices;
        MeshSourceBounds bounds{};
    };
} // namespace Zenith
