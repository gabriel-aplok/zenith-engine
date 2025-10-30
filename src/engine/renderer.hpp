#pragma once
#include "engine/shader.hpp"
#include <glad/glad.h>
#include <vector>

namespace Zenith {

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        // I will add more later: uv, normal, etc.
    };

    class Renderer {
    public:
        Renderer();
        ~Renderer();

        void beginScene();
        void endScene();
        void draw() const;

        void submit(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, Shader &shader);

    private:
        GLuint m_vao = 0, m_vbo = 0, m_ebo = 0;
        std::vector<Vertex> m_vertexBuffer;
        std::vector<uint32_t> m_indexBuffer;
    };

} // namespace Zenith