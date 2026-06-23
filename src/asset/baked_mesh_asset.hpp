#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace Zenith {

    struct BakedMeshAsset {
        std::vector<float> vertices;
        std::vector<std::uint32_t> indices;

        std::size_t vertexFloatCount() const { return vertices.size(); }
        std::size_t vertexCount() const { return vertices.size() / 6; }
        std::size_t indexCount() const { return indices.size(); }
        bool empty() const { return vertices.empty() || indices.empty(); }
    };

} // namespace Zenith
