#include "scene/transform.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Zenith
{
    Transform::Transform()
    {
        rebuildMatrix();
    }

    void Transform::setPosition(const glm::vec3 &position)
    {
        m_position = position;
        rebuildMatrix();
    }

    void Transform::setRotation(const glm::quat &rotation)
    {
        m_rotation = glm::normalize(rotation);
        rebuildMatrix();
    }

    void Transform::setRotationEulerRadians(const glm::vec3 &eulerRadians)
    {
        setRotation(glm::quat(eulerRadians));
    }

    void Transform::setRotationEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        setRotationEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::setScale(const glm::vec3 &scale)
    {
        m_scale = scale;
        rebuildMatrix();
    }

    void Transform::translate(const glm::vec3 &delta)
    {
        m_position += delta;
        rebuildMatrix();
    }

    void Transform::rotate(const glm::quat &deltaRotation)
    {
        m_rotation = glm::normalize(deltaRotation * m_rotation);
        rebuildMatrix();
    }

    void Transform::rotateEulerRadians(const glm::vec3 &eulerRadians)
    {
        rotate(glm::quat(eulerRadians));
    }

    void Transform::rotateEulerDegrees(const glm::vec3 &eulerDegrees)
    {
        rotateEulerRadians(glm::radians(eulerDegrees));
    }

    void Transform::rescale(const glm::vec3 &deltaScale)
    {
        m_scale += deltaScale;
        rebuildMatrix();
    }

    void Transform::rebuildMatrix()
    {
        const glm::mat4 translation = glm::translate(glm::mat4{1.0f}, m_position);
        const glm::mat4 rotation = glm::mat4_cast(m_rotation);
        const glm::mat4 scaling = glm::scale(glm::mat4{1.0f}, m_scale);
        m_localToWorld = translation * rotation * scaling;
    }
} // namespace Zenith
