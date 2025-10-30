#include "engine/shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

namespace Zenith {

    std::string Shader::readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open shader: " + path);
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::cout << buffer.str();
        return buffer.str();
    }

    Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
        std::string vertexSource = readFile(vertexPath);
        std::string fragmentSource = readFile(fragmentPath);
        compile(vertexSource, fragmentSource);
    }

    Shader::~Shader() {
        if (m_id) {
            glDeleteProgram(m_id);
        }
    }

    void Shader::bind() const {
        glUseProgram(m_id);
    }

    void Shader::unbind() const {
        glUseProgram(0);
    }

    void Shader::compile(const std::string& vertexSource, const std::string& fragmentSource) {
        auto compileShader = [](GLenum type, const std::string& source) -> GLuint {
            GLuint shader = glCreateShader(type);
            const char* src = source.c_str();
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);

            GLint success;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(shader, 512, nullptr, infoLog);
                throw std::runtime_error(std::string("Shader compile failed:\n") + infoLog);
            }
            return shader;
            };

        GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
        GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

        m_id = glCreateProgram();
        glAttachShader(m_id, vertex);
        glAttachShader(m_id, fragment);
        glLinkProgram(m_id);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        GLint success;
        glGetProgramiv(m_id, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(m_id, 512, nullptr, infoLog);
            throw std::runtime_error(std::string("Program link failed:\n") + infoLog);
        }
    }

    // Uniform setters
    void Shader::setBool(const std::string& name, bool value) const {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), static_cast<int>(value));
    }

    void Shader::setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void Shader::setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
    }

    void Shader::setVec2(const std::string& name, const glm::vec2& value) const {
        glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void Shader::setVec4(const std::string& name, const glm::vec4& value) const {
        glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, &value[0]);
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& value) const {
        glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, &value[0][0]);
    }

} // namespace Zenith