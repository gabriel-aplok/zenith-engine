#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

namespace Zenith {

    class Shader {
    public:
        Shader(const std::string &vertexPath, const std::string &fragmentPath);
        ~Shader();

        void bind() const;
        void unbind() const;

        void setBool(const std::string &name, bool value) const;
        void setInt(const std::string &name, int value) const;
        void setFloat(const std::string &name, float value) const;
        void setVec2(const std::string &name, const glm::vec2 &value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setVec4(const std::string &name, const glm::vec4 &value) const;
        void setMat4(const std::string &name, const glm::mat4 &value) const;
    private:
        void compile(const std::string &vertexSource, const std::string &fragmentSource);
        std::string readFile(const std::string &path);

        GLuint m_id = 0;
    };

} // namespace Zenith
