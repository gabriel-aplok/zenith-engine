#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Zenith
{
    class Transform
    {
    public:
        Transform();

        const glm::vec3 &position() const { return m_position; }
        const glm::quat &rotation() const { return m_rotation; }
        const glm::vec3 &scale() const { return m_scale; }

        void setPosition(const glm::vec3 &position);
        void setRotation(const glm::quat &rotation);
        void setRotationEulerRadians(const glm::vec3 &eulerRadians);
        void setRotationEulerDegrees(const glm::vec3 &eulerDegrees);
        void setScale(const glm::vec3 &scale);

        void translate(const glm::vec3 &delta);
        void rotate(const glm::quat &deltaRotation);
        void rotateEulerRadians(const glm::vec3 &eulerRadians);
        void rotateEulerDegrees(const glm::vec3 &eulerDegrees);
        void rescale(const glm::vec3 &deltaScale);

        const glm::mat4 &localToWorld() const { return m_localToWorld; }
        const glm::mat4 &worldMatrix() const { return m_localToWorld; }

    private:
        void rebuildMatrix();

        glm::vec3 m_position{0.0f};
        glm::quat m_rotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 m_scale{1.0f};
        glm::mat4 m_localToWorld{1.0f};
    };
} // namespace Zenith
