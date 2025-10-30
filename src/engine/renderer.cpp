#include "engine/renderer.hpp"
#include <stdexcept>

namespace Zenith {

    Renderer::Renderer() {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);

        // position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    Renderer::~Renderer() {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }

    void Renderer::beginScene() {
        m_vertexBuffer.clear();
        m_indexBuffer.clear();
    }

    void Renderer::submit(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices, Shader &shader) {
        m_vertexBuffer.insert(m_vertexBuffer.end(), vertices.begin(), vertices.end());
        m_indexBuffer.insert(m_indexBuffer.end(), indices.begin(), indices.end());
    }

    void Renderer::endScene() {
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertexBuffer.size() * sizeof(Vertex), m_vertexBuffer.data(), GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer.size() * sizeof(uint32_t), m_indexBuffer.data(), GL_DYNAMIC_DRAW);

        glBindVertexArray(0);
    }

    void Renderer::draw() const {
        glBindVertexArray(m_vao);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indexBuffer.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

} // namespace Zenith