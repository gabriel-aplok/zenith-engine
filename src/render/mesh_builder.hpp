#pragma once

#include <cstdint>
#include <vector>

#include "render/mesh.hpp"

namespace Zenith::MeshBuilder
{

  Render::Bounds computeBounds(const std::vector<Render::MeshVertex> &vertices);
  Render::MeshData makeMesh(std::vector<Render::MeshVertex> vertices, std::vector<uint16_t> indices);
  Render::MeshData makePyramid();
  Render::MeshData makeCube();
  Render::MeshData makePlane();

} // namespace Zenith::MeshBuilder
