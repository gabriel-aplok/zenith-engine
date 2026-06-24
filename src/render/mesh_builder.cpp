#include "render/mesh_builder.hpp"

#include <limits>

namespace Zenith::MeshBuilder
{
    Render::Bounds computeBounds(const std::vector<Render::MeshVertex> &vertices)
    {
        if (vertices.empty())
        {
            return {};
        }

        glm::vec3 minPoint{std::numeric_limits<float>::max()};
        glm::vec3 maxPoint{std::numeric_limits<float>::lowest()};
        for (const auto &vertex : vertices)
        {
            minPoint = glm::min(minPoint, vertex.position);
            maxPoint = glm::max(maxPoint, vertex.position);
        }

        Render::Bounds bounds{};
        bounds.center = (minPoint + maxPoint) * 0.5f;
        bounds.extents = (maxPoint - minPoint) * 0.5f;
        return bounds;
    }

    Render::MeshData makeMesh(std::vector<Render::MeshVertex> vertices, std::vector<uint16_t> indices)
    {
        Render::MeshData mesh;
        mesh.bounds = computeBounds(vertices);
        mesh.vertices = std::move(vertices);
        mesh.indices = std::move(indices);
        return mesh;
    }

    Render::MeshData makePyramid()
    {
        return makeMesh({
                            {{0.0f, 1.0f, 0.0f}, {0.5f, 1.0f}, {1.0f, 0.2f, 0.2f, 1.0f}},
                            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}, {0.2f, 1.0f, 0.2f, 1.0f}},
                            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {0.2f, 0.2f, 1.0f, 1.0f}},
                            {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 0.2f, 1.0f}},
                            {{-1.0f, -1.0f, -1.0f}, {0.0f, 1.0f}, {1.0f, 0.2f, 1.0f, 1.0f}},
                        },
                        {
                            0,
                            2,
                            1,
                            0,
                            3,
                            2,
                            0,
                            4,
                            3,
                            0,
                            1,
                            4,
                            1,
                            2,
                            3,
                            1,
                            3,
                            4,
                        });
    }

    Render::MeshData makeCube()
    {
        return makeMesh({
                            {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.2f, 0.2f, 1.0f}},
                            {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f}, {0.2f, 1.0f, 0.2f, 1.0f}},
                            {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}, {0.2f, 0.2f, 1.0f, 1.0f}},
                            {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 1.0f, 0.2f, 1.0f}},
                            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f}, {1.0f, 0.2f, 1.0f, 1.0f}},
                            {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f}, {0.2f, 1.0f, 1.0f, 1.0f}},
                            {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f}, {0.9f, 0.9f, 0.9f, 1.0f}},
                            {{-1.0f, 1.0f, -1.0f}, {1.0f, 1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}},
                        },
                        {
                            0,
                            1,
                            2,
                            0,
                            2,
                            3,
                            1,
                            5,
                            6,
                            1,
                            6,
                            2,
                            5,
                            4,
                            7,
                            5,
                            7,
                            6,
                            4,
                            0,
                            3,
                            4,
                            3,
                            7,
                            3,
                            2,
                            6,
                            3,
                            6,
                            7,
                            4,
                            5,
                            1,
                            4,
                            1,
                            0,
                        });
    }

    Render::MeshData makePlane()
    {
        return makeMesh({
                            {{-1.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {0.9f, 0.9f, 0.9f, 1.0f}},
                            {{1.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {0.7f, 0.8f, 1.0f, 1.0f}},
                            {{1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.7f, 1.0f, 0.8f, 1.0f}},
                            {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.8f, 0.7f, 1.0f}},
                        },
                        {
                            0,
                            1,
                            2,
                            0,
                            2,
                            3,
                        });
    }

} // namespace Zenith::MeshBuilder
