#pragma once

#include <string>

#include "render/mesh.hpp"

namespace Zenith
{
    struct BakedMeshAsset
    {
        Render::MeshData mesh;
        std::string sourcePath;
    };
} // namespace Zenith
