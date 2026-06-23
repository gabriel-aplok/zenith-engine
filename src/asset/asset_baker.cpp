#include "asset/asset_baker.hpp"

#include "asset/baked_mesh_asset.hpp"
#include "log/log.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <fstream>
#include <string>
#include <unordered_map>

namespace {

    struct AssetHeader {
        std::uint64_t vertexCount = 0;
        std::uint64_t indexCount = 0;
    };

    void writeBlob(std::ofstream& out, const void* data, std::size_t size) {
        out.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    }

    struct IndexHash {
        std::size_t operator()(const tinyobj::index_t& idx) const {
            const std::size_t h1 = std::hash<int>{}(idx.vertex_index);
            const std::size_t h2 = std::hash<int>{}(idx.normal_index);
            const std::size_t h3 = std::hash<int>{}(idx.texcoord_index);
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    struct IndexEqual {
        bool operator()(const tinyobj::index_t& a, const tinyobj::index_t& b) const {
            return a.vertex_index == b.vertex_index &&
                   a.normal_index == b.normal_index &&
                   a.texcoord_index == b.texcoord_index;
        }
    };

    bool extractObjMesh(const tinyobj::attrib_t& attrib,
                        const std::vector<tinyobj::shape_t>& shapes,
                        Zenith::BakedMeshAsset& asset) {
        std::unordered_map<tinyobj::index_t, std::uint32_t, IndexHash, IndexEqual> uniqueVerts;

        for (const auto& shape : shapes) {
            std::size_t idxOffset = 0;
            for (std::size_t face = 0; face < shape.mesh.num_face_vertices.size(); ++face) {
                const int faceVertexCount = shape.mesh.num_face_vertices[face];

                if (faceVertexCount != 3) {
                    Zenith::Log::Warn("OBJ contains non-triangle face, skipping");
                    idxOffset += faceVertexCount;
                    continue;
                }

                for (int vertex = 0; vertex < faceVertexCount; ++vertex) {
                    const tinyobj::index_t index = shape.mesh.indices[idxOffset + vertex];

                    if (!uniqueVerts.contains(index)) {
                        const auto newIndex = static_cast<std::uint32_t>(asset.vertices.size() / 6);

                        if (index.vertex_index >= 0 &&
                            static_cast<std::size_t>(index.vertex_index * 3 + 2) < attrib.vertices.size()) {
                            asset.vertices.insert(asset.vertices.end(),
                                {
                                    attrib.vertices[3 * index.vertex_index + 0],
                                    attrib.vertices[3 * index.vertex_index + 1],
                                    attrib.vertices[3 * index.vertex_index + 2]
                                });
                        } else {
                            asset.vertices.insert(asset.vertices.end(), {0.0f, 0.0f, 0.0f});
                        }

                        if (index.normal_index >= 0 &&
                            static_cast<std::size_t>(index.normal_index * 3 + 2) < attrib.normals.size()) {
                            asset.vertices.insert(asset.vertices.end(),
                                {
                                    attrib.normals[3 * index.normal_index + 0],
                                    attrib.normals[3 * index.normal_index + 1],
                                    attrib.normals[3 * index.normal_index + 2]
                                });
                        } else {
                            asset.vertices.insert(asset.vertices.end(), {0.0f, 0.0f, 0.0f});
                        }

                        uniqueVerts[index] = newIndex;
                    }

                    asset.indices.push_back(uniqueVerts[index]);
                }

                idxOffset += faceVertexCount;
            }
        }

        return !asset.empty();
    }

} // namespace

bool AssetBaker::bakeModelToAsset(const std::filesystem::path& sourcePath,
                                  const std::filesystem::path& destinationPath) {
    const std::string extension = sourcePath.extension().string();
    Zenith::BakedMeshAsset asset;

    if (extension == ".obj") {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn;
        std::string err;

        const bool loaded = tinyobj::LoadObj(
            &attrib,
            &shapes,
            &materials,
            &warn,
            &err,
            sourcePath.string().c_str(),
            nullptr,
            true);

        if (!warn.empty()) {
            Zenith::Log::Warn("OBJ warning: {}", warn);
        }

        if (!err.empty()) {
            Zenith::Log::Error("OBJ error: {}", err);
            return false;
        }

        if (!loaded) {
            Zenith::Log::Error("Failed to load OBJ {}", sourcePath.string());
            return false;
        }

        if (!extractObjMesh(attrib, shapes, asset)) {
            Zenith::Log::Error("Failed to extract mesh data from OBJ");
            return false;
        }
    } else {
        Zenith::Log::Error("Unsupported source format: {}", extension);
        return false;
    }

    if (asset.empty()) {
        Zenith::Log::Error("No mesh data extracted from {}", sourcePath.string());
        return false;
    }

    std::ofstream out(destinationPath, std::ios::binary);
    if (!out.is_open()) {
        Zenith::Log::Error("Cannot open destination file: {}", destinationPath.string());
        return false;
    }

    const AssetHeader header{
        .vertexCount = static_cast<std::uint64_t>(asset.vertices.size()),
        .indexCount = static_cast<std::uint64_t>(asset.indices.size())
    };

    writeBlob(out, &header, sizeof(header));
    writeBlob(out, asset.vertices.data(), asset.vertices.size() * sizeof(float));
    writeBlob(out, asset.indices.data(), asset.indices.size() * sizeof(std::uint32_t));

    Zenith::Log::Info("Baked asset: {} -> {} ({} vertices, {} indices)",
                      sourcePath.filename().string(),
                      destinationPath.filename().string(),
                      asset.vertexCount(),
                      asset.indexCount());
    return true;
}
