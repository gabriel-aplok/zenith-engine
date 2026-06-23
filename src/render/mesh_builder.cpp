#include "render/mesh_builder.hpp"

namespace Zenith::MeshBuilder
{

    Render::MeshData makePyramid()
    {
        Render::MeshData mesh;
        mesh.vertices = {
            {{0.0f, 1.0f, 0.0f}, {1.0f, 0.2f, 0.2f, 1.0f}},
            {{-1.0f, -1.0f, 1.0f}, {0.2f, 1.0f, 0.2f, 1.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.2f, 0.2f, 1.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 0.2f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.2f, 1.0f, 1.0f}},
        };

        mesh.indices = {
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
        };
        return mesh;
    }

    Render::MeshData makeCube()
    {
        Render::MeshData mesh;
        mesh.vertices = {
            {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.2f, 0.2f, 1.0f}},
            {{1.0f, -1.0f, 1.0f}, {0.2f, 1.0f, 0.2f, 1.0f}},
            {{1.0f, 1.0f, 1.0f}, {0.2f, 0.2f, 1.0f, 1.0f}},
            {{-1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 0.2f, 1.0f}},
            {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.2f, 1.0f, 1.0f}},
            {{1.0f, -1.0f, -1.0f}, {0.2f, 1.0f, 1.0f, 1.0f}},
            {{1.0f, 1.0f, -1.0f}, {0.9f, 0.9f, 0.9f, 1.0f}},
            {{-1.0f, 1.0f, -1.0f}, {0.4f, 0.4f, 0.4f, 1.0f}},
        };

        mesh.indices = {
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
        };
        return mesh;
    }

} // namespace Zenith::MeshBuilder
