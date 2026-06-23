#pragma once

#include <filesystem>
#include <optional>

#include "render/mesh.hpp"

namespace Zenith
{
    std::optional<Render::MeshData> loadObjMesh(const std::filesystem::path &path);
} // namespace Zenith
