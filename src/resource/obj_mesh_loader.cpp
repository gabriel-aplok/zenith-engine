#include "resource/obj_mesh_loader.hpp"

#include <fstream>
#include <sstream>
#include <string>

#include "render/mesh_builder.hpp"

namespace Zenith
{
    namespace
    {
        int resolveObjIndex(int index, std::size_t count)
        {
            if (index > 0)
            {
                return index - 1;
            }

            if (index < 0)
            {
                return static_cast<int>(count) + index;
            }

            return -1;
        }

        bool parseFaceVertex(const std::string &token, int &vertexIndex, int &texCoordIndex)
        {
            const std::size_t slash = token.find('/');
            const std::string indexText = slash == std::string::npos ? token : token.substr(0, slash);
            if (indexText.empty())
            {
                return false;
            }

            try
            {
                vertexIndex = std::stoi(indexText);
                texCoordIndex = -1;
                if (slash != std::string::npos)
                {
                    const std::size_t secondSlash = token.find('/', slash + 1);
                    const std::string texCoordText = token.substr(slash + 1, secondSlash == std::string::npos ? std::string::npos : secondSlash - slash - 1);
                    if (!texCoordText.empty())
                    {
                        texCoordIndex = std::stoi(texCoordText);
                    }
                }
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    } // namespace

    std::optional<Render::MeshData> loadObjMesh(const std::filesystem::path &path)
    {
        std::ifstream file(path);
        if (!file)
        {
            return std::nullopt;
        }

        std::vector<Vector3> positions;
        std::vector<Vector2> texCoords;
        std::vector<Render::MeshVertex> vertices;
        std::vector<uint16_t> indices;

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream stream(line);
            std::string kind;
            stream >> kind;
            if (kind.empty() || kind[0] == '#')
            {
                continue;
            }

            if (kind == "v")
            {
                float x = 0.0f;
                float y = 0.0f;
                float z = 0.0f;
                if (stream >> x >> y >> z)
                {
                    positions.emplace_back(x, y, z);
                }
                continue;
            }

            if (kind == "vt")
            {
                float u = 0.0f;
                float v = 0.0f;
                if (stream >> u >> v)
                {
                    texCoords.emplace_back(u, 1.0f - v);
                }
                continue;
            }

            if (kind == "f")
            {
                std::vector<uint16_t> faceIndices;
                std::string token;
                while (stream >> token)
                {
                    int objIndex = 0;
                    int texCoordIndex = -1;
                    if (!parseFaceVertex(token, objIndex, texCoordIndex))
                    {
                        continue;
                    }

                    const int resolved = resolveObjIndex(objIndex, positions.size());
                    if (resolved < 0 || resolved >= static_cast<int>(positions.size()))
                    {
                        continue;
                    }

                    const Vector3 position = positions[static_cast<std::size_t>(resolved)];
                    Vector2 uv{(position.x + 1.0f) * 0.5f, (position.z + 1.0f) * 0.5f};
                    if (texCoordIndex > 0)
                    {
                        const int resolvedUv = resolveObjIndex(texCoordIndex, texCoords.size());
                        if (resolvedUv >= 0 && resolvedUv < static_cast<int>(texCoords.size()))
                        {
                            uv = texCoords[static_cast<std::size_t>(resolvedUv)];
                        }
                    }
                    vertices.push_back(Render::MeshVertex{position, uv, Vector4{1.0f}});
                    faceIndices.push_back(static_cast<uint16_t>(vertices.size() - 1));
                }

                if (faceIndices.size() >= 3)
                {
                    const uint16_t first = faceIndices[0];
                    for (std::size_t i = 1; i + 1 < faceIndices.size(); ++i)
                    {
                        indices.push_back(first);
                        indices.push_back(faceIndices[i]);
                        indices.push_back(faceIndices[i + 1]);
                    }
                }
            }
        }

        if (vertices.empty() || indices.empty())
        {
            return std::nullopt;
        }

        Render::MeshData mesh;
        mesh.vertices = std::move(vertices);
        mesh.indices = std::move(indices);
        mesh.bounds = MeshBuilder::computeBounds(mesh.vertices);
        return mesh;
    }
} // namespace Zenith
